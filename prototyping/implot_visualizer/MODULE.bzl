module(
    name = "implot_vis_test",
    repo_name = "implot_vis_test_src",
    version = "0.9",
)

bazel_dep(name = "aspect_bazel_lib", version = "2.9.3")
bazel_dep(name = "yaml-cpp", version = "0.8.0")
bazel_dep(name = "spdlog", version = "1.11.0")