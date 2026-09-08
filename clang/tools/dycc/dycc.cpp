//===- dycc.cpp - Dayan compiler driver -----------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include <algorithm>
#include <cerrno>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <optional>
#include <set>
#include <string>
#include <system_error>
#include <utility>
#include <vector>

#include "clang/Dayan/Dayan.h"

#include <llvm/ADT/SmallString.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/MathExtras.h>
#include <llvm/Support/Path.h>
#include <llvm/Support/Program.h>

#ifndef DYCC_SOURCE_ROOT
#define DYCC_SOURCE_ROOT "."
#endif

namespace fs = std::filesystem;
namespace {

static constexpr const char *DayanDeviceTriple = "riscv64-unknown-elf";
static constexpr const char *DayanDeviceArch = "rv64imaf_zicsr_zifencei";
static constexpr uint32_t NpuPackageVersion = 5U;
static constexpr uint32_t NpuPackageHeaderSize = 128U;
static constexpr uint32_t NpuPackageKernelVersion = 5U;
static constexpr uint32_t NpuPackageKernelSize = 96U;
static constexpr uint32_t NpuPackageArgumentSize = 16U;
static constexpr uint32_t NpuPackageTaskModeSingleCore = 1U << 0;
static constexpr uint32_t NpuPackageTaskModeFourCore = 1U << 1;
static constexpr uint32_t NpuPackageTaskModeAll =
    NpuPackageTaskModeSingleCore | NpuPackageTaskModeFourCore;
static constexpr uint64_t NpuPackageProcStackSize = 0x10000U;
static constexpr uint64_t NpuPackageProcPrintfSize = 0x1000U;

struct Options {
  fs::path input;
  fs::path output;
  fs::path root;
  fs::path host_cxx = "clang++";
  fs::path device_cxx = "clang++";
  fs::path device_linker = "ld.lld";
  fs::path objcopy = "llvm-objcopy";
  fs::path save_temps_dir;
  fs::path emit_host;
  fs::path emit_device;
  fs::path emit_package;
  std::string temp_stem;
  std::vector<std::string> common_args;
  std::vector<std::string> host_args;
  std::vector<std::string> device_args;
  std::vector<fs::path> source_inputs;
  std::vector<std::string> link_args;
  bool save_temps = false;
  bool split_only = false;
  bool compile_only = false;
  bool dry_run = false;
  bool verbose = false;
};

static void print_usage(std::ostream &out) {
  out << "usage: dycc [options] -c input.cpp [-o output.o]\n"
         "       dycc [options] input.cpp... [objects/libraries] [-o program]\n"
         "\n"
         "Dayan compiler-driver options:\n"
         "  -c                       compile to Host object without linking\n"
         "  --dayan-root <dir>       repository/SDK root\n"
         "  --host-cxx <path>        Host clang++ (default: clang++)\n"
         "  --device-cxx <path>      Device clang++ (default: clang++)\n"
         "  --device-linker <path>   Device linker (default: ld.lld)\n"
         "  -Xhost <arg>             pass one argument only to Host clang\n"
         "  -Xdevice <arg>           pass one argument only to Device clang\n"
         "  -Xlinker <arg>           pass one argument to the Host linker\n"
         "  --save-temps[=<dir>]     retain generated sources and objects\n"
         "  --emit-host <path>       also write the split Host source\n"
         "  --emit-device <path>     also write the split Device source\n"
         "  --emit-package <path>    also write a standalone Dayan package\n"
         "                           (recommended suffix: .dypkg)\n"
         "  --split-only             stop after source generation\n"
         "  --dry-run                print compiler commands without running\n"
         "  -v, --verbose            print executed compiler commands\n"
         "  --version                print dycc version\n"
         "\n"
         "Without -c, dycc links a Host executable with libDayanRuntime.\n"
         "Unrecognised compiler options are forwarded to both clang passes.\n";
}

static void append_u32(std::vector<unsigned char> &out, uint32_t value) {
  for (unsigned shift = 0; shift != 32; shift += 8)
    out.push_back(static_cast<unsigned char>((value >> shift) & 0xffU));
}

static void append_u64(std::vector<unsigned char> &out, uint64_t value) {
  for (unsigned shift = 0; shift != 64; shift += 8)
    out.push_back(static_cast<unsigned char>((value >> shift) & 0xffU));
}

static void append_zeros(std::vector<unsigned char> &out, std::size_t count) {
  out.insert(out.end(), count, 0);
}

static void append_bytes(std::vector<unsigned char> &out,
                         llvm::StringRef bytes) {
  out.insert(out.end(), bytes.begin(), bytes.end());
}

static void align_to(std::vector<unsigned char> &out, std::size_t alignment) {
  if (alignment == 0)
    return;
  std::size_t padding = (alignment - (out.size() % alignment)) % alignment;
  append_zeros(out, padding);
}

static bool checked_u32(uint64_t value, uint32_t &result,
                        const char *field_name) {
  if (value > std::numeric_limits<uint32_t>::max()) {
    std::cerr << "dycc: error: package " << field_name
              << " does not fit in 32 bits\n";
    return false;
  }
  result = static_cast<uint32_t>(value);
  return true;
}

static bool starts_with(const std::string &value, const std::string &prefix) {
  return value.compare(0, prefix.size(), prefix) == 0;
}

static fs::path device_api_include(const fs::path &root) {
  fs::path source_tree = root / "device/include/api";
  return fs::is_directory(source_tree) ? source_tree : root / "include/api";
}

static bool is_dayan_root(const fs::path &root) {
  return fs::is_directory(device_api_include(root)) &&
         fs::is_directory(root / "host/include") &&
         fs::is_regular_file(root / "host/dycc/include/dayan_compiler.h");
}

static fs::path default_dayan_root(const char *argv0) {
  std::error_code error;
  fs::path executable = fs::read_symlink("/proc/self/exe", error);
  if (error && argv0 && fs::path(argv0).has_parent_path()) {
    error.clear();
    executable = fs::weakly_canonical(argv0, error);
  }
  if (!error && !executable.empty()) {
    fs::path prefix = executable.parent_path().parent_path();
    const fs::path candidates[] = {
        prefix / "lib/dayan",
        prefix / "lib64/dayan",
        prefix / "share/dayan",
    };
    for (const fs::path &candidate : candidates) {
      if (is_dayan_root(candidate))
        return candidate;
    }
  }
  return fs::path(DYCC_SOURCE_ROOT);
}

static std::optional<std::string> take_value(int &index, int argc, char **argv,
                                             const std::string &option) {
  if (index + 1 >= argc) {
    std::cerr << "dycc: error: missing value after " << option << "\n";
    return std::nullopt;
  }
  return std::string(argv[++index]);
}

static bool compiler_option_takes_value(const std::string &arg) {
  static const std::set<std::string> options = {
      "-I",
      "-D",
      "-U",
      "-include",
      "-imacros",
      "-isystem",
      "-iquote",
      "-idirafter",
      "-isysroot",
      "--sysroot",
      "-x",
      "-std",
      "-stdlib",
      "-resource-dir",
      "-gcc-toolchain",
      "--gcc-toolchain",
      "-fdebug-prefix-map",
      "-ffile-prefix-map",
      "-fmacro-prefix-map",
      "-MF",
      "-MT",
      "-MQ",
  };
  return options.count(arg) != 0;
}

static bool is_link_input(const fs::path &path) {
  std::string name = path.filename().string();
  std::string extension = path.extension().string();
  return extension == ".o" || extension == ".obj" || extension == ".a" ||
         extension == ".so" || extension == ".dylib" ||
         name.find(".so.") != std::string::npos;
}

static bool is_link_only_option(const std::string &arg) {
  return arg == "-L" || arg == "-l" || arg == "-T" || arg == "-u" ||
         arg == "-z" || arg == "-Xlinker" || arg == "-static" ||
         arg == "-static-libgcc" || arg == "-static-libstdc++" ||
         arg == "-pie" || arg == "-no-pie" || arg == "-rdynamic" ||
         arg == "-s" || starts_with(arg, "-L") || starts_with(arg, "-l") ||
         starts_with(arg, "-Wl,") || starts_with(arg, "-fuse-ld=");
}

static bool link_option_takes_value(const std::string &arg) {
  return arg == "-L" || arg == "-l" || arg == "-T" || arg == "-u" ||
         arg == "-z" || arg == "-Xlinker";
}

static bool parse_options(int argc, char **argv, Options &options) {
  std::vector<fs::path> inputs;
  bool positional_only = false;

  options.root = default_dayan_root(argc > 0 ? argv[0] : nullptr);
  if (const char *root = std::getenv("DAYAN_ROOT"))
    options.root = root;
  if (const char *host = std::getenv("DYCC_HOST_CXX"))
    options.host_cxx = host;
  if (const char *device = std::getenv("DYCC_DEVICE_CXX"))
    options.device_cxx = device;

  for (int i = 1; i < argc; ++i) {
    std::string arg = argv[i];
    if (!positional_only && (arg == "-h" || arg == "--help")) {
      print_usage(std::cout);
      std::exit(0);
    }
    if (!positional_only && arg == "--version") {
      std::cout << "dycc 0.1 (Dayan clang driver)\n";
      std::exit(0);
    }
    if (!positional_only && arg == "--") {
      positional_only = true;
      continue;
    }
    if (!positional_only && arg == "-c") {
      options.compile_only = true;
      continue;
    }
    if (!positional_only && (arg == "-v" || arg == "--verbose")) {
      options.verbose = true;
      continue;
    }
    if (!positional_only && arg == "--dry-run") {
      options.dry_run = true;
      continue;
    }
    if (!positional_only && arg == "--split-only") {
      options.split_only = true;
      continue;
    }
    if (!positional_only && arg == "--save-temps") {
      options.save_temps = true;
      continue;
    }
    if (!positional_only && starts_with(arg, "--save-temps=")) {
      options.save_temps = true;
      options.save_temps_dir = arg.substr(std::strlen("--save-temps="));
      continue;
    }
    auto path_option = [&](const char *name, fs::path &destination) {
      if (arg != name)
        return false;
      auto value = take_value(i, argc, argv, arg);
      if (!value) {
        destination.clear();
        return true;
      }
      destination = *value;
      return true;
    };
    if (!positional_only && path_option("--dayan-root", options.root)) {
      if (options.root.empty())
        return false;
      continue;
    }
    if (!positional_only && path_option("--host-cxx", options.host_cxx)) {
      if (options.host_cxx.empty())
        return false;
      continue;
    }
    if (!positional_only && path_option("--device-cxx", options.device_cxx)) {
      if (options.device_cxx.empty())
        return false;
      continue;
    }
    if (!positional_only &&
        path_option("--device-linker", options.device_linker)) {
      if (options.device_linker.empty())
        return false;
      continue;
    }
    if (!positional_only && path_option("--objcopy", options.objcopy)) {
      if (options.objcopy.empty())
        return false;
      continue;
    }
    if (!positional_only && path_option("--emit-host", options.emit_host)) {
      if (options.emit_host.empty())
        return false;
      continue;
    }
    if (!positional_only && path_option("--emit-device", options.emit_device)) {
      if (options.emit_device.empty())
        return false;
      continue;
    }
    if (!positional_only &&
        path_option("--emit-package", options.emit_package)) {
      if (options.emit_package.empty())
        return false;
      continue;
    }
    if (!positional_only && (arg == "-o" || arg == "--output")) {
      auto value = take_value(i, argc, argv, arg);
      if (!value)
        return false;
      options.output = *value;
      continue;
    }
    if (!positional_only && starts_with(arg, "-o") && arg.size() > 2) {
      options.output = arg.substr(2);
      continue;
    }
    if (!positional_only && (arg == "-Xhost" || arg == "-Xdevice")) {
      auto value = take_value(i, argc, argv, arg);
      if (!value)
        return false;
      (arg == "-Xhost" ? options.host_args : options.device_args)
          .push_back(*value);
      continue;
    }
    if (!positional_only && is_link_only_option(arg)) {
      options.link_args.push_back(arg);
      if (link_option_takes_value(arg)) {
        auto value = take_value(i, argc, argv, arg);
        if (!value)
          return false;
        options.link_args.push_back(*value);
      }
      continue;
    }
    if (!positional_only && starts_with(arg, "--host-arg=")) {
      options.host_args.push_back(arg.substr(std::strlen("--host-arg=")));
      continue;
    }
    if (!positional_only && starts_with(arg, "--device-arg=")) {
      options.device_args.push_back(arg.substr(std::strlen("--device-arg=")));
      continue;
    }
    if (!positional_only && !arg.empty() && arg[0] == '-') {
      if (arg == "-S" || arg == "-E" || arg == "-shared" || arg == "--target" ||
          starts_with(arg, "--target=") || arg == "-target" ||
          starts_with(arg, "-march=") || starts_with(arg, "-mabi=")) {
        std::cerr << "dycc: error: unsupported or fixed driver option '" << arg
                  << "'\n";
        return false;
      }
      options.common_args.push_back(arg);
      if (compiler_option_takes_value(arg)) {
        auto value = take_value(i, argc, argv, arg);
        if (!value)
          return false;
        options.common_args.push_back(*value);
      }
      continue;
    }
    if (is_link_input(arg))
      options.link_args.push_back(arg);
    else
      inputs.emplace_back(arg);
  }

  if (inputs.empty()) {
    std::cerr << "dycc: error: at least one input source is required\n";
    return false;
  }
  options.source_inputs = std::move(inputs);
  if (options.compile_only && !options.link_args.empty()) {
    std::cerr << "dycc: error: linker inputs/options cannot be used with -c\n";
    return false;
  }
  if ((options.split_only || !options.emit_host.empty() ||
       !options.emit_device.empty()) &&
      options.source_inputs.size() != 1) {
    std::cerr << "dycc: error: split/emit options require exactly one "
                 "input source\n";
    return false;
  }
  if (!options.emit_package.empty() && options.compile_only &&
      options.source_inputs.size() != 1) {
    std::cerr << "dycc: error: --emit-package with -c currently requires "
                 "exactly one input source\n";
    return false;
  }
  if (options.split_only && !options.emit_package.empty()) {
    std::cerr << "dycc: error: --emit-package cannot be used with "
                 "--split-only\n";
    return false;
  }
  if (options.compile_only && options.source_inputs.size() != 1 &&
      !options.output.empty()) {
    std::cerr << "dycc: error: cannot use -o with -c and multiple inputs\n";
    return false;
  }
  if (options.output.empty() && !options.split_only) {
    if (options.compile_only && options.source_inputs.size() == 1) {
      options.output = options.source_inputs.front().filename();
      options.output.replace_extension(".o");
    } else if (!options.compile_only) {
      options.output = "a.out";
    }
  }
  if (options.split_only && !options.save_temps && options.emit_host.empty() &&
      options.emit_device.empty()) {
    std::cerr << "dycc: error: --split-only needs --save-temps or an "
                 "--emit-* output\n";
    return false;
  }
  return true;
}

static std::optional<std::string> find_program(const fs::path &program);

static std::vector<std::string> dayan_device_target_args() {
  return {
      "--target=" + std::string(DayanDeviceTriple),
      "-march=" + std::string(DayanDeviceArch),
      "-mabi=lp64",
      "-Xclang",
      "-target-feature",
      "-Xclang",
      "+txlsu",
      "-Xclang",
      "-target-feature",
      "-Xclang",
      "+txmat",
      "-Xclang",
      "-target-feature",
      "-Xclang",
      "+txvec",
      "-Xclang",
      "-target-feature",
      "-Xclang",
      "+txbase",
      "-Xclang",
      "-target-feature",
      "-Xclang",
      "+txmacro",
      "-Wno-ignored-attributes",
  };
}

static std::optional<std::string> clang_resource_dir(const fs::path &clangxx) {
  auto executable = find_program(clangxx);
  if (!executable)
    return std::nullopt;

  llvm::SmallString<256> output_path;
  if (llvm::sys::fs::createTemporaryFile("dycc-res", "txt", output_path))
    return std::nullopt;

  std::string output_file = std::string(output_path.str());
  std::string error;
  bool execution_failed = false;
  const llvm::StringRef args[] = {*executable, "-print-resource-dir"};
  const std::optional<llvm::StringRef> redirects[] = {
      std::nullopt, llvm::StringRef(output_file), std::nullopt};
  int status =
      llvm::sys::ExecuteAndWait(*executable, args, std::nullopt, redirects, 0,
                                0, &error, &execution_failed);
  if (execution_failed || status != 0) {
    llvm::sys::fs::remove(output_file);
    return std::nullopt;
  }

  std::ifstream stream(output_file);
  std::string line;
  std::getline(stream, line);
  llvm::sys::fs::remove(output_file);
  while (!line.empty() && (line.back() == '\n' || line.back() == '\r'))
    line.pop_back();
  if (line.empty())
    return std::nullopt;
  return line;
}

static bool split_source(const Options &options,
                         clang::dayan::SplitOutput &output) {
  std::error_code error;
  fs::path absolute = fs::absolute(options.input, error);
  if (error) {
    std::cerr << "dycc: error: cannot resolve input path: " << error.message()
              << "\n";
    return false;
  }
  clang::dayan::SplitOptions split_options;
  split_options.InputPath = absolute.string();
  split_options.WorkingDirectory = fs::current_path().string();
  split_options.CompilerAttributesHeader =
      (options.root / "host/dycc/include/dayan_compiler.h").string();
  split_options.DeviceTriple = DayanDeviceTriple;
  split_options.DeviceArch = DayanDeviceArch;
  split_options.DeviceAPIInclude = device_api_include(options.root).string();
  split_options.HostInclude = (options.root / "host/include").string();
  split_options.CommonArgs = options.common_args;
  split_options.DeviceArgs = options.device_args;
  if (auto resource_dir = clang_resource_dir(options.device_cxx))
    split_options.CompilerResourceDir = *resource_dir;
  return clang::dayan::splitSource(split_options, output);
}

static bool write_file(const fs::path &path, llvm::StringRef contents) {
  std::error_code error;
  if (!path.parent_path().empty())
    fs::create_directories(path.parent_path(), error);
  if (error) {
    std::cerr << "dycc: error: cannot create directory for '" << path
              << "': " << error.message() << "\n";
    return false;
  }
  std::ofstream stream(path, std::ios::binary | std::ios::trunc);
  if (!stream) {
    std::cerr << "dycc: error: cannot write '" << path << "'\n";
    return false;
  }
  stream.write(contents.data(), static_cast<std::streamsize>(contents.size()));
  if (!stream) {
    std::cerr << "dycc: error: failed while writing '" << path << "'\n";
    return false;
  }
  return true;
}

static bool write_file(const fs::path &path,
                       const std::vector<unsigned char> &contents) {
  return write_file(
      path, llvm::StringRef(reinterpret_cast<const char *>(contents.data()),
                            contents.size()));
}

static std::string quote_command_arg(const std::string &arg) {
  if (arg.find_first_of(" \t\n'\"\\$`") == std::string::npos)
    return arg;
  std::string result = "'";
  for (char c : arg) {
    if (c == '\'')
      result += "'\\''";
    else
      result.push_back(c);
  }
  result.push_back('\'');
  return result;
}

static std::optional<std::string> find_program(const fs::path &program) {
  if (program.has_parent_path()) {
    std::error_code error;
    fs::path absolute = fs::absolute(program, error);
    if (!error && fs::exists(absolute))
      return absolute.string();
    return std::nullopt;
  }
  auto found = llvm::sys::findProgramByName(program.string());
  if (!found)
    return std::nullopt;
  return *found;
}

static int run_command(const fs::path &program,
                       const std::vector<std::string> &arguments,
                       const Options &options) {
  auto executable = find_program(program);
  if (!executable) {
    std::cerr << "dycc: error: cannot find executable '" << program.string()
              << "'\n";
    return 1;
  }
  std::vector<std::string> storage;
  storage.reserve(arguments.size() + 1);
  storage.push_back(*executable);
  storage.insert(storage.end(), arguments.begin(), arguments.end());
  if (options.verbose || options.dry_run) {
    std::cerr << "dycc:";
    for (const std::string &arg : storage)
      std::cerr << ' ' << quote_command_arg(arg);
    std::cerr << "\n";
  }
  if (options.dry_run)
    return 0;
  std::vector<llvm::StringRef> refs;
  refs.reserve(storage.size());
  for (const std::string &arg : storage)
    refs.emplace_back(arg);
  std::string error;
  bool failed = false;
  int status = llvm::sys::ExecuteAndWait(*executable, refs, std::nullopt, {}, 0,
                                         0, &error, &failed);
  if (failed || status < 0) {
    std::cerr << "dycc: error: failed to execute '" << program.string() << "'";
    if (!error.empty())
      std::cerr << ": " << error;
    std::cerr << "\n";
    return 1;
  }
  return status;
}

static bool read_binary(const fs::path &path,
                        std::vector<unsigned char> &data) {
  std::ifstream stream(path, std::ios::binary);
  if (!stream) {
    std::cerr << "dycc: error: cannot read '" << path << "'\n";
    return false;
  }
  stream.seekg(0, std::ios::end);
  std::streamoff length = stream.tellg();
  stream.seekg(0, std::ios::beg);
  if (length <= 0 || static_cast<std::uintmax_t>(length) >
                         std::numeric_limits<std::size_t>::max()) {
    std::cerr << "dycc: error: invalid Device object size\n";
    return false;
  }
  data.resize(static_cast<std::size_t>(length));
  stream.read(reinterpret_cast<char *>(data.data()), length);
  return static_cast<bool>(stream);
}

static bool
build_package(const std::vector<unsigned char> &object,
              const std::vector<clang::dayan::PackageKernelInfo> &kernels,
              std::vector<unsigned char> &package) {
  if (kernels.empty()) {
    std::cerr << "dycc: error: cannot emit a package without __kernel__ "
                 "functions\n";
    return false;
  }

  uint64_t argument_count_64 = 0;
  for (const auto &kernel : kernels)
    argument_count_64 += kernel.Arguments.size();

  uint32_t kernel_count = 0;
  uint32_t argument_count = 0;
  if (!checked_u32(kernels.size(), kernel_count, "kernel count") ||
      !checked_u32(argument_count_64, argument_count, "argument count"))
    return false;

  const uint64_t object_offset = NpuPackageHeaderSize;
  const uint64_t object_size = object.size();
  const uint64_t kernel_offset =
      llvm::alignTo(object_offset + object_size, uint64_t(8));
  const uint64_t argument_offset =
      kernel_offset + uint64_t(NpuPackageKernelSize) * kernel_count;
  const uint64_t sync_offset =
      argument_offset + uint64_t(NpuPackageArgumentSize) * argument_count;
  const uint64_t string_offset = sync_offset;

  std::vector<unsigned char> strings;
  std::vector<uint32_t> name_offsets;
  std::vector<uint32_t> entry_offsets;
  name_offsets.reserve(kernels.size());
  entry_offsets.reserve(kernels.size());

  auto add_string = [&](const std::string &value,
                        const char *field_name) -> std::optional<uint32_t> {
    uint32_t offset = 0;
    if (!checked_u32(string_offset + strings.size(), offset, field_name))
      return std::nullopt;
    strings.insert(strings.end(), value.begin(), value.end());
    strings.push_back('\0');
    return offset;
  };

  for (const auto &kernel : kernels) {
    auto name = add_string(kernel.Name, "kernel name offset");
    auto entry = add_string(kernel.EntrySymbol, "kernel entry offset");
    if (!name || !entry)
      return false;
    name_offsets.push_back(*name);
    entry_offsets.push_back(*entry);
  }
  auto isa_string = add_string(DayanDeviceArch, "ISA string offset");
  if (!isa_string)
    return false;

  uint32_t isa_string_size = 0;
  if (!checked_u32(std::strlen(DayanDeviceArch) + 1, isa_string_size,
                   "ISA string size"))
    return false;

  const uint64_t total_size = string_offset + strings.size();

  package.clear();
  append_bytes(package, "DAYANPKG");
  append_u32(package, NpuPackageVersion);
  append_u32(package, NpuPackageHeaderSize);
  append_u64(package, total_size);
  append_u32(package, 0); // flags
  append_u32(package, 0); // target ABI
  append_u64(package, object_offset);
  append_u64(package, object_size);
  append_u64(package, kernel_offset);
  append_u32(package, kernel_count);
  append_u32(package, 0);
  append_u64(package, argument_offset);
  append_u32(package, argument_count);
  append_u32(package, 0);
  append_u64(package, sync_offset);
  append_u32(package, 0); // sync count
  append_u32(package, 0);
  append_u64(package, string_offset);
  append_u64(package, strings.size());
  append_u32(package, *isa_string);
  append_u32(package, isa_string_size);
  append_u64(package, 0);
  if (package.size() != NpuPackageHeaderSize) {
    std::cerr << "dycc: error: internal package header size mismatch\n";
    return false;
  }

  package.insert(package.end(), object.begin(), object.end());
  align_to(package, 8);
  if (package.size() != kernel_offset) {
    std::cerr << "dycc: error: internal package object layout mismatch\n";
    return false;
  }

  uint32_t first_argument = 0;
  for (std::size_t index = 0; index < kernels.size(); ++index) {
    const auto &kernel = kernels[index];
    uint32_t kernel_argument_count = 0;
    if (!checked_u32(kernel.Arguments.size(), kernel_argument_count,
                     "kernel argument count"))
      return false;

    std::size_t before = package.size();
    append_u32(package, NpuPackageKernelVersion);
    append_u32(package, name_offsets[index]);
    append_u32(package, entry_offsets[index]);
    append_u32(package, NpuPackageTaskModeAll);
    append_u32(package, first_argument);
    append_u32(package, kernel_argument_count);
    append_u32(package, 0); // first sync
    append_u32(package, 0); // sync count
    append_u32(package, kernel.MaxTasksPerCore);
    append_u32(package, kernel.MaxChildThreads);
    append_u32(package, kernel.Flags);
    append_u32(package, 0);
    append_u64(package, NpuPackageProcStackSize);
    append_u64(package, kernel.AtomicCacheSize);
    append_u64(package, NpuPackageProcPrintfSize);
    append_u64(package, 0); // resident memory
    append_u64(package, kernel.UramSize);
    package.push_back(static_cast<unsigned char>(kernel.LsuTcsrCount));
    package.push_back(static_cast<unsigned char>(kernel.MatTcsrCount));
    package.push_back(static_cast<unsigned char>(kernel.VecTcsrCount));
    package.push_back(static_cast<unsigned char>(kernel.MacroTcsrCount));
    append_u32(package, 0);
    if (package.size() - before != NpuPackageKernelSize) {
      std::cerr << "dycc: error: internal package kernel size mismatch\n";
      return false;
    }
    first_argument += kernel_argument_count;
  }

  if (package.size() != argument_offset) {
    std::cerr << "dycc: error: internal package kernel layout mismatch\n";
    return false;
  }
  for (const auto &kernel : kernels) {
    for (const auto &argument : kernel.Arguments) {
      append_u32(package, argument.Kind);
      append_u32(package, argument.Size);
      append_u32(package, argument.Alignment);
      append_u32(package, argument.Access);
    }
  }
  if (package.size() != string_offset) {
    std::cerr << "dycc: error: internal package argument layout mismatch\n";
    return false;
  }
  package.insert(package.end(), strings.begin(), strings.end());
  if (package.size() != total_size) {
    std::cerr << "dycc: error: internal package size mismatch\n";
    return false;
  }
  return true;
}

static std::string embedded_source(const std::vector<unsigned char> &data) {
  static const char hex[] = "0123456789abcdef";
  std::string result;
  result.reserve(data.size() * 6 + 512);
  result += "#include <stddef.h>\n"
            "extern \"C\" {\n"
            "__attribute__((aligned(16), visibility(\"hidden\"), "
            "section(\".rodata.dayan.kernel\")))\n"
            "extern const unsigned char __kernel_obj__[] = {\n";
  for (std::size_t i = 0; i < data.size(); ++i) {
    if (i % 12 == 0)
      result += "    ";
    result += "0x";
    result.push_back(hex[data[i] >> 4]);
    result.push_back(hex[data[i] & 15]);
    result += i + 1 == data.size() ? "\n" : ", ";
    if (i % 12 == 11 && i + 1 != data.size())
      result += "\n";
  }
  result +=
      "};\n"
      "__attribute__((visibility(\"hidden\")))\n"
      "extern const size_t __kernel_obj_size__ = sizeof(__kernel_obj__);\n"
      "}\n";
  return result;
}

static fs::path make_temp_directory() {
  fs::path base;
  if (const char *temporary = std::getenv("TMPDIR"))
    base = temporary;
  else
    base = "/tmp";
  std::string pattern = (base / "dycc-XXXXXX").string();
  std::vector<char> buffer(pattern.begin(), pattern.end());
  buffer.push_back('\0');
  char *created = ::mkdtemp(buffer.data());
  return created ? fs::path(created) : fs::path();
}

static std::vector<std::string> device_compile_args(const Options &options,
                                                    const fs::path &source,
                                                    const fs::path &object) {
  std::vector<std::string> args = {
      "-x",
      "c++",
      "-std=c++17",
  };
  {
    auto target = dayan_device_target_args();
    args.insert(args.end(), target.begin(), target.end());
  }
  args.insert(args.end(),
              {
                  "-mcmodel=medany",
                  "-mno-relax",
                  "-msmall-data-limit=0",
                  "-ffreestanding",
                  "-fno-builtin",
                  "-fno-common",
                  "-fno-pic",
                  "-fno-pie",
                  "-fno-stack-protector",
                  "-fno-unwind-tables",
                  "-fno-asynchronous-unwind-tables",
                  "-fno-exceptions",
                  "-fno-rtti",
                  "-fno-threadsafe-statics",
                  "-fno-use-cxa-atexit",
                  "-ffunction-sections",
                  "-fdata-sections",
                  "-O2",
                  "-D__DAYAN_DEVICE__=1",
                  "-D__host__=",
                  "-D__device__=__attribute__((used))",
                  "-D__kernel__=__attribute__((used,visibility(\"default\")))",
                  "-isystem",
                  device_api_include(options.root).string(),
                  "-isystem",
                  (options.root / "host/include").string(),
              });
  if (auto resource_dir = clang_resource_dir(options.device_cxx)) {
    args.insert(args.end(), {"-resource-dir", *resource_dir});
  }
  args.insert(args.end(), options.common_args.begin(),
              options.common_args.end());
  args.insert(args.end(), options.device_args.begin(),
              options.device_args.end());
  args.insert(args.end(), {"-c", source.string(), "-o", object.string()});
  return args;
}

static std::vector<std::string> host_compile_args(const Options &options,
                                                  const fs::path &source,
                                                  const fs::path &object) {
  std::vector<std::string> args = {
      "-x",
      "c++",
      "-std=c++17",
      "-O2",
      "-D__DAYAN_HOST__=1",
      "-D__host__=",
      "-D__device__=",
      "-D__kernel__=",
      "-isystem",
      (options.root / "host/include").string(),
  };
  args.insert(args.end(), options.common_args.begin(),
              options.common_args.end());
  args.insert(args.end(), options.host_args.begin(), options.host_args.end());
  args.insert(args.end(), {"-c", source.string(), "-o", object.string()});
  return args;
}

static fs::path temp_path(const Options &options, const fs::path &work,
                          const std::string &suffix) {
  std::string stem = options.temp_stem.empty() ? options.input.filename().string()
                                               : options.temp_stem;
  return work / (stem + suffix);
}

struct CompiledUnit {
  fs::path host_object;
  fs::path device_object;
  std::vector<clang::dayan::PackageKernelInfo> kernels;
};

static int compile_split_unit(const Options &options, const fs::path &work,
                              CompiledUnit &unit) {
  clang::dayan::SplitOutput split;
  if (!split_source(options, split))
    return 1;

  fs::path host_source = temp_path(options, work, ".dycc.host.cpp");
  fs::path device_source = temp_path(options, work, ".dycc.device.cpp");
  unit.host_object = temp_path(options, work, ".dycc.host.o");
  unit.device_object = temp_path(options, work, ".dycc.device.o");
  unit.kernels = std::move(split.Kernels);

  if (!write_file(host_source, split.Host) ||
      !write_file(device_source, split.Device))
    return 1;
  if (!options.emit_host.empty() && !write_file(options.emit_host, split.Host))
    return 1;
  if (!options.emit_device.empty() && !write_file(options.emit_device, split.Device))
    return 1;
  if (options.split_only)
    return 0;

  if (run_command(options.device_cxx,
                  device_compile_args(options, device_source, unit.device_object),
                  options) != 0)
    return 1;
  if (run_command(options.host_cxx,
                  host_compile_args(options, host_source, unit.host_object),
                  options) != 0)
    return 1;
  return 0;
}

static bool compile_embed_object(const Options &options,
                                 const std::vector<unsigned char> &image,
                                 const fs::path &work,
                                 const fs::path &embed_object) {
  fs::path embed_source = temp_path(options, work, ".dycc.embed.cpp");
  return write_file(embed_source, embedded_source(image)) &&
         run_command(options.host_cxx,
                     host_compile_args(options, embed_source, embed_object),
                     options) == 0;
}

static int link_device_objects(const Options &options,
                               const std::vector<fs::path> &device_objects,
                               const fs::path &output) {
  std::vector<std::string> args = {
      "-m",
      "elf64lriscv",
      "-r",
  };
  for (const fs::path &object : device_objects)
    args.push_back(object.string());
  args.insert(args.end(), {"-o", output.string()});
  return run_command(options.device_linker, args, options);
}

static int compile_object(const Options &options) {
  if (!fs::is_regular_file(options.input)) {
    std::cerr << "dycc: error: input is not a regular file: '" << options.input
              << "'\n";
    return 1;
  }
  if (!is_dayan_root(options.root)) {
    std::cerr << "dycc: error: invalid Dayan root '" << options.root << "'\n";
    return 1;
  }

  fs::path temporary;
  fs::path work;
  if (options.save_temps) {
    work = options.save_temps_dir.empty()
               ? fs::absolute(options.output.empty() ? fs::path(".")
                                                     : options.output)
                     .parent_path()
               : options.save_temps_dir;
    std::error_code error;
    fs::create_directories(work, error);
    if (error) {
      std::cerr << "dycc: error: cannot create temporary output directory: "
                << error.message() << "\n";
      return 1;
    }
  } else {
    temporary = make_temp_directory();
    if (temporary.empty()) {
      std::cerr << "dycc: error: cannot create temporary directory: "
                << std::strerror(errno) << "\n";
      return 1;
    }
    work = temporary;
  }

  struct Cleanup {
    fs::path path;
    ~Cleanup() {
      if (!path.empty()) {
        std::error_code ignored;
        fs::remove_all(path, ignored);
      }
    }
  } cleanup{temporary};

  CompiledUnit unit;
  if (compile_split_unit(options, work, unit) != 0)
    return 1;
  if (options.split_only)
    return 0;
  if (options.dry_run) {
    std::cerr << "dycc: note: embed and final-link commands require the "
                 "Device object and are omitted in --dry-run mode\n";
    return 0;
  }

  std::vector<unsigned char> image;
  if (!read_binary(unit.device_object, image))
    return 1;
  if (!options.emit_package.empty()) {
    std::vector<unsigned char> package;
    if (!build_package(image, unit.kernels, package) ||
        !write_file(options.emit_package, package))
      return 1;
  }
  fs::path embed_object = temp_path(options, work, ".dycc.embed.o");
  if (!compile_embed_object(options, image, work, embed_object))
    return 1;

  std::error_code error;
  if (!options.output.parent_path().empty())
    fs::create_directories(options.output.parent_path(), error);
  if (error) {
    std::cerr << "dycc: error: cannot create output directory: "
              << error.message() << "\n";
    return 1;
  }
  std::vector<std::string> link_args = {
      "-r",
      "-nostdlib",
      unit.host_object.string(),
      embed_object.string(),
      "-o",
      options.output.string(),
  };
  if (run_command(options.host_cxx, link_args, options) != 0)
    return 1;
  std::vector<std::string> localize_args = {
      "--localize-symbol=__kernel_obj__",
      "--localize-symbol=__kernel_obj_size__",
      options.output.string(),
  };
  if (run_command(options.objcopy, localize_args, options) != 0)
    return 1;
  return 0;
}

static std::optional<fs::path> find_runtime_archive(const Options &options) {
  if (const char *override_path = std::getenv("DYCC_RUNTIME_LIBRARY")) {
    fs::path path = override_path;
    if (fs::is_regular_file(path))
      return path;
    std::cerr << "dycc: error: DYCC_RUNTIME_LIBRARY is not a regular "
                 "file: '"
              << path << "'\n";
    return std::nullopt;
  }

  fs::path prefix = options.root.parent_path().parent_path();
  const fs::path candidates[] = {
      options.root / "build/host/driver/libDayanRuntime.a",
      options.root.parent_path() / "libDayanRuntime.a",
      prefix / "lib/libDayanRuntime.a",
      prefix / "lib64/libDayanRuntime.a",
  };
  for (const fs::path &candidate : candidates) {
    if (fs::is_regular_file(candidate))
      return candidate;
  }
  std::cerr << "dycc: error: cannot find libDayanRuntime.a; build the Host "
               "runtime or set DYCC_RUNTIME_LIBRARY\n";
  return std::nullopt;
}

static bool collect_kernel_metadata(
    const CompiledUnit &unit,
    std::set<std::string> &kernel_names,
    std::vector<clang::dayan::PackageKernelInfo> &kernels) {
  for (const auto &kernel : unit.kernels) {
    if (!kernel_names.insert(kernel.Name).second) {
      std::cerr << "dycc: error: duplicate __kernel__ name across translation "
                   "units: '"
                << kernel.Name << "'\n";
      return false;
    }
    kernels.push_back(kernel);
  }
  return true;
}

static int link_executable(const Options &options,
                           const std::vector<fs::path> &objects) {
  auto runtime = find_runtime_archive(options);
  if (!runtime)
    return 1;

  std::error_code error;
  if (!options.output.parent_path().empty())
    fs::create_directories(options.output.parent_path(), error);
  if (error) {
    std::cerr << "dycc: error: cannot create output directory: "
              << error.message() << "\n";
    return 1;
  }

  std::vector<std::string> arguments;
  arguments.insert(arguments.end(), options.common_args.begin(),
                   options.common_args.end());
  arguments.insert(arguments.end(), options.host_args.begin(),
                   options.host_args.end());
  for (const fs::path &object : objects)
    arguments.push_back(object.string());
  arguments.insert(arguments.end(), options.link_args.begin(),
                   options.link_args.end());
  arguments.insert(arguments.end(), {
                                        runtime->string(),
                                        "-pthread",
                                        "-o",
                                        options.output.string(),
                                    });
  return run_command(options.host_cxx, arguments, options);
}

static int run_driver(const Options &options) {
  if (options.split_only) {
    Options unit = options;
    unit.input = options.source_inputs.front();
    return compile_object(unit);
  }

  if (options.compile_only) {
    for (const fs::path &source : options.source_inputs) {
      Options unit = options;
      unit.input = source;
      if (options.source_inputs.size() == 1) {
        unit.output = options.output;
      } else {
        unit.output = source.filename();
        unit.output.replace_extension(".o");
      }
      if (compile_object(unit) != 0)
        return 1;
    }
    return 0;
  }

  fs::path temporary;
  fs::path work;
  if (options.save_temps) {
    work = options.save_temps_dir.empty()
               ? fs::absolute(options.output).parent_path()
               : options.save_temps_dir;
    std::error_code error;
    fs::create_directories(work, error);
    if (error) {
      std::cerr << "dycc: error: cannot create link work directory: "
                << error.message() << "\n";
      return 1;
    }
  } else {
    temporary = make_temp_directory();
    if (temporary.empty()) {
      std::cerr << "dycc: error: cannot create temporary directory: "
                << std::strerror(errno) << "\n";
      return 1;
    }
    work = temporary;
  }

  struct Cleanup {
    fs::path path;
    ~Cleanup() {
      if (!path.empty()) {
        std::error_code ignored;
        fs::remove_all(path, ignored);
      }
    }
  } cleanup{temporary};

  std::vector<fs::path> host_objects;
  std::vector<fs::path> device_objects;
  std::vector<clang::dayan::PackageKernelInfo> kernels;
  std::set<std::string> kernel_names;
  host_objects.reserve(options.source_inputs.size() + 1);
  device_objects.reserve(options.source_inputs.size());
  for (std::size_t index = 0; index < options.source_inputs.size(); ++index) {
    const fs::path &source = options.source_inputs[index];
    Options unit_options = options;
    unit_options.input = source;
    if (options.source_inputs.size() != 1)
      unit_options.temp_stem =
          source.filename().string() + "." + std::to_string(index);
    unit_options.output = work / ((unit_options.temp_stem.empty()
                                      ? source.filename().string()
                                      : unit_options.temp_stem) +
                                  ".dycc.o");
    CompiledUnit unit;
    if (compile_split_unit(unit_options, work, unit) != 0)
      return 1;
    if (!collect_kernel_metadata(unit, kernel_names, kernels))
      return 1;
    host_objects.push_back(unit.host_object);
    device_objects.push_back(unit.device_object);
  }

  if (options.dry_run) {
    std::cerr << "dycc: note: embed and Host link commands require the linked "
                 "Device object and are omitted in --dry-run mode\n";
    return 0;
  }

  fs::path linked_device_object = work / "linked.dayan.device.o";
  if (link_device_objects(options, device_objects, linked_device_object) != 0)
    return 1;

  std::vector<unsigned char> image;
  if (!read_binary(linked_device_object, image))
    return 1;
  if (!options.emit_package.empty()) {
    std::vector<unsigned char> package;
    if (!build_package(image, kernels, package) ||
        !write_file(options.emit_package, package))
      return 1;
  }

  fs::path embed_object = work / "linked.dayan.embed.o";
  if (!compile_embed_object(options, image, work, embed_object))
    return 1;
  host_objects.push_back(embed_object);
  return link_executable(options, host_objects);
}

} // namespace

int main(int argc, char **argv) {
  Options options;
  if (!parse_options(argc, argv, options)) {
    print_usage(std::cerr);
    return 1;
  }
  return run_driver(options);
}
