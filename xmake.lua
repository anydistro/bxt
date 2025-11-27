-- ╔══════════════════════════════════════════════════════════════════════════════╗
-- ║                              🏗️  PROJECT SETUP                               ║
-- ║                          Basic project configuration                         ║
-- ╚══════════════════════════════════════════════════════════════════════════════╝

set_project("bxt")
set_version("1.0.0")
set_languages("c++23")

-- add_repositories("local-repo " .. path.absolute("./xmake")) -- Local repository for custom packages

-- ──────────────────────────────────────────────────────────────────────────────
-- ⚙️  Build options and rules
-- ──────────────────────────────────────────────────────────────────────────────
option("testing")
    set_default(false)
    set_showmenu(true)
    set_description("Enable testing")
option_end()

add_rules("mode.debug", "mode.release")

-- ──────────────────────────────────────────────────────────────────────────────
-- 🛠️  Global compiler configuration
-- ──────────────────────────────────────────────────────────────────────────────
add_cxxflags("-Wall", "-Wextra", "-Werror")
set_toolchains("clang")
set_runtimes("c++_static")
add_cxxflags("-stdlib=libc++")
add_ldflags("-stdlib=libc++", "-lc++abi")

-- ╔══════════════════════════════════════════════════════════════════════════════╗
-- ║                             📦 DEPENDENCIES                                  ║
-- ║                        Package requirements and configs                      ║
-- ╚══════════════════════════════════════════════════════════════════════════════╝

add_requires("openssl", "fmt", "frozen", "yaml-cpp", "jwt-cpp", "cpp-httplib")
add_requires("parallel-hashmap", "cli11", "boost_di")
add_requires("spdlog", "date", "mariadb-connector-c", {configs = {mysqlcompat = true}})
add_requires("reflect-cpp")

add_requires("libcoro", {configs = {networking = true}})
add_requires("sqlgen", {configs = {sqlite3 = true, debug = true, mysql = true}})
add_requires("cpptrace", {configs = {unwind = "libunwind"}})
add_requires("drogon v1.9.11", {configs = {spdlog = true}})
add_requireconfs("drogon.trantor", {override = true, version = "v1.5.24"})


add_requires("boost", {configs = {
    iostreams = true,
    filesystem = true,
    system = true,
}})

add_requires("libarchive", {configs = {
    zstd = true,
    bzip2 = true,
    lzo = true,
}})

-- ──────────────────────────────────────────────────────────────────────────────
-- 🧪 Testing dependencies
-- ──────────────────────────────────────────────────────────────────────────────
if has_config("testing") then
    add_requires("catch2 >=3.7.0")
end

-- ╔══════════════════════════════════════════════════════════════════════════════╗
-- ║                           🚀 MAIN EXECUTABLE                                 ║
-- ║                          Primary application target                          ║
-- ╚══════════════════════════════════════════════════════════════════════════════╝

target("bxtnext")
    set_kind("binary")
    set_default(true)
    
    -- ──────────────────────────────────────────────────────────────────────────
    -- 📁 Source files configuration
    -- ──────────────────────────────────────────────────────────────────────────
    add_files("vertical/src/**.cpp")
    add_headerfiles("vertical/src/**.hpp")

    remove_files("vertical/src/**/*_test.cpp")
    remove_files("vertical/src/**/*_disabled/**")
    remove_files("vertical/src/percpptency/example.cpp")
    remove_files("vertical/src/signal/signal_tracer.cpp") -- Exclude signal_tracer from main build
    
    -- ──────────────────────────────────────────────────────────────────────────
    -- 📂 Include directories
    -- ──────────────────────────────────────────────────────────────────────────
    add_includedirs("vertical/src")
    add_includedirs("cpperr/include")
    
    -- ──────────────────────────────────────────────────────────────────────────
    -- 📦 Package dependencies
    -- ──────────────────────────────────────────────────────────────────────────
    add_packages("openssl", "fmt", "frozen", "yaml-cpp", "jwt-cpp", "cpp-httplib")
    add_packages("parallel-hashmap", "trantor", "drogon", "libcoro", "cli11", "boost_di")
    add_packages("spdlog", "date", "boost", "libarchive", "sqlgen", "cpptrace", "mariadb-connector-c")
    add_packages("reflect-cpp")
    
    -- ──────────────────────────────────────────────────────────────────────────
    -- ⚙️  Build configuration
    -- ──────────────────────────────────────────────────────────────────────────
    add_defines("REFLECTCPP_USE_STD_EXPECTED")
    set_targetdir("build/bin")

-- ╔══════════════════════════════════════════════════════════════════════════════╗
-- ║                              🧪 TESTING                                      ║
-- ║                           Test executable target                            ║
-- ╚══════════════════════════════════════════════════════════════════════════════╝

if has_config("testing") then
    target("bxtnext_tests")
        set_kind("binary")
        
        -- ──────────────────────────────────────────────────────────────────────
        -- 📁 Test source files configuration  
        -- ──────────────────────────────────────────────────────────────────────
        add_files("vertical/src/**.cpp")
        add_headerfiles("vertical/src/**.hpp")
        
        remove_files("vertical/src/**/*_disabled/**")
        remove_files("vertical/src/signal/signal_tracer.cpp") -- Exclude signal_tracer from tests
        
        -- ──────────────────────────────────────────────────────────────────────
        -- 📂 Test include directories
        -- ──────────────────────────────────────────────────────────────────────
        add_includedirs("vertical/src")
        add_includedirs("cpperr/include")
        
        -- ──────────────────────────────────────────────────────────────────────
        -- 📦 Test package dependencies
        -- ──────────────────────────────────────────────────────────────────────
        add_packages("catch2", "openssl", "fmt", "frozen", "yaml-cpp", "jwt-cpp")
        add_packages("cpp-httplib", "parallel-hashmap", "drogon", "libcoro", "cli11")
        add_packages("boost_di", "sqlite3", "spdlog", "date", "boost", "libarchive", "sqlgen", "cpptrace")
        
        -- ──────────────────────────────────────────────────────────────────────
        -- ⚙️  Test build configuration
        -- ──────────────────────────────────────────────────────────────────────
        set_targetdir("build/bin/tests")
end

-- ╔══════════════════════════════════════════════════════════════════════════════╗
-- ║                         🔍 SIGNAL TRACER                                     ║
-- ║                    Signal-safe stack trace resolver                          ║
-- ╚══════════════════════════════════════════════════════════════════════════════╝

target("signal_tracer")
    set_kind("binary")
    
    -- ──────────────────────────────────────────────────────────────────────────
    -- 📁 Signal tracer source files
    -- ──────────────────────────────────────────────────────────────────────────
    add_files("vertical/src/signal/signal_tracer.cpp")
    
    -- ──────────────────────────────────────────────────────────────────────────
    -- 📦 Signal tracer package dependencies
    -- ──────────────────────────────────────────────────────────────────────────
    add_packages("cpptrace")
    
    -- ──────────────────────────────────────────────────────────────────────────
    -- ⚙️  Signal tracer build configuration
    -- ──────────────────────────────────────────────────────────────────────────
    set_targetdir("build/bin")
