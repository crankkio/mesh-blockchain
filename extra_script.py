from os.path import join
Import("env")

# Add library source files to build
env.BuildSources(
    join("$BUILD_DIR", "lib_crypto"),
    join("$PROJECT_DIR", ".pio", "libdeps", "native", "Crypto"),
)

env.BuildSources(
    join("$BUILD_DIR", "lib_base64"),
    join("$PROJECT_DIR", ".pio", "libdeps", "native", "base64_encode", "src"),
)

# Add mock files
env.BuildSources(
    join("$BUILD_DIR", "mock"),
    join("$PROJECT_DIR", "test", "mock"),
)
