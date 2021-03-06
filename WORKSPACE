load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")
load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository")

CEL_SPEC_GIT_SHA = "b154461b3a037f9654852087ef96be2b756871a0"  # 10/16/2019

CEL_SPEC_SHA = "a88cf903fc890cb8e53048365d05a5c0c03e35148b03812de7a471d7d2ff8744"

http_archive(
    name = "com_google_cel_spec",
    sha256 = CEL_SPEC_SHA,
    strip_prefix = "cel-spec-" + CEL_SPEC_GIT_SHA,
    urls = ["https://github.com/google/cel-spec/archive/" + CEL_SPEC_GIT_SHA + ".zip"],
)

http_archive(
    name = "com_google_absl",
    strip_prefix = "abseil-cpp-master",
    urls = ["https://github.com/abseil/abseil-cpp/archive/master.zip"],
)

# Google RE2 (Regular Expression) C++ Library
http_archive(
    name = "com_googlesource_code_re2",
    strip_prefix = "re2-master",
    urls = ["https://github.com/google/re2/archive/master.zip"],
)

# gRPC dependencies:
http_archive(
    name = "com_github_grpc_grpc",
    sha256 = "ffbe61269160ea745e487f79b0fd06b6edd3d50c6d9123f053b5634737cf2f69",
    strip_prefix = "grpc-1.25.0",
    urls = ["https://github.com/grpc/grpc/archive/v1.25.0.tar.gz"],
)

load("@com_github_grpc_grpc//bazel:grpc_deps.bzl", "grpc_deps")

grpc_deps()

load("@upb//bazel:repository_defs.bzl", "bazel_version_repository")

bazel_version_repository(
    name = "bazel_version",
)

load("@build_bazel_rules_apple//apple:repositories.bzl", "apple_rules_dependencies")

apple_rules_dependencies()

load("@build_bazel_apple_support//lib:repositories.bzl", "apple_support_dependencies")

apple_support_dependencies()

GOOGLEAPIS_GIT_SHA = "be480e391cc88a75cf2a81960ef79c80d5012068"  # Jul 24, 2019

GOOGLEAPIS_SHA = "c1969e5b72eab6d9b6cfcff748e45ba57294aeea1d96fd04cd081995de0605c2"

http_archive(
    name = "com_google_googleapis",
    sha256 = GOOGLEAPIS_SHA,
    strip_prefix = "googleapis-" + GOOGLEAPIS_GIT_SHA,
    urls = ["https://github.com/googleapis/googleapis/archive/" + GOOGLEAPIS_GIT_SHA + ".tar.gz"],
)

load("@com_google_googleapis//:repository_rules.bzl", "switched_rules_by_language")

switched_rules_by_language(
    name = "com_google_googleapis_imports",
    cc = True,
    go = True,
    grpc = True,
)

http_archive(
    name = "io_bazel_rules_go",
    sha256 = "a82a352bffae6bee4e95f68a8d80a70e87f42c4741e6a448bec11998fcc82329",
    url = "https://github.com/bazelbuild/rules_go/releases/download/0.18.5/rules_go-0.18.5.tar.gz",
)

load("@io_bazel_rules_go//go:deps.bzl", "go_rules_dependencies", "go_register_toolchains")

# cel-go dependencies:
http_archive(
    name = "bazel_gazelle",
    sha256 = "3c681998538231a2d24d0c07ed5a7658cb72bfb5fd4bf9911157c0e9ac6a2687",
    urls = ["https://github.com/bazelbuild/bazel-gazelle/releases/download/0.17.0/bazel-gazelle-0.17.0.tar.gz"],
)

load("@bazel_gazelle//:deps.bzl", "gazelle_dependencies", "go_repository")

git_repository(
    name = "com_google_cel_go",
    remote = "https://github.com/google/cel-go.git",
    tag = "v0.2.0",
)

go_repository(
    name = "org_golang_google_genproto",
    build_file_proto_mode = "disable",
    commit = "bd91e49a0898e27abb88c339b432fa53d7497ac0",
    importpath = "google.golang.org/genproto",
)

go_repository(
    name = "com_github_antlr",
    importpath = "github.com/antlr/antlr4",
    tag = "4.7.2",
)

go_rules_dependencies()

go_register_toolchains()

gazelle_dependencies()

# Parser dependencies
http_archive(
    name = "rules_antlr",
    urls = ["https://github.com/marcohu/rules_antlr/archive/3cc2f9502a54ceb7b79b37383316b23c4da66f9a.tar.gz"],
    strip_prefix = "rules_antlr-3cc2f9502a54ceb7b79b37383316b23c4da66f9a",
    sha256 = "7249d1569293d9b239e23c65f6b4c81a07da921738bde0dfeb231ed98be40429",
)

load("@rules_antlr//antlr:deps.bzl", "antlr_dependencies")
antlr_dependencies(472)

http_archive(
    name = "antlr4_runtimes",
    sha256 = "46f5e1af5f4bd28ade55cb632f9a069656b31fc8c2408f9aa045f9b5f5caad64",
    strip_prefix = "antlr4-4.7.2",
    urls = ["https://github.com/antlr/antlr4/archive/4.7.2.tar.gz"],
    build_file_content = """
package(default_visibility = ["//visibility:public"])
cc_library(
    name = "cpp",
    srcs = glob(["runtime/Cpp/runtime/src/**/*.cpp"]),
    hdrs = glob(["runtime/Cpp/runtime/src/**/*.h"]),
    includes = ["runtime/Cpp/runtime/src"],
)
"""
)

# tools/flatbuffers dependencies
FLAT_BUFFERS_SHA = "a83caf5910644ba1c421c002ef68e42f21c15f9f"

http_archive(
    name = "com_github_google_flatbuffers",
    sha256 = "b8efbc25721e76780752bad775a97c3f77a0250271e2db37fc747b20e8b0f24a",
    strip_prefix = "flatbuffers-" + FLAT_BUFFERS_SHA,
    url = "https://github.com/google/flatbuffers/archive/" + FLAT_BUFFERS_SHA + ".tar.gz",
)
