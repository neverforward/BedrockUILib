add_rules("mode.debug", "mode.release")

add_repositories("levimc-repo https://github.com/LiteLDev/xmake-repo.git")

option("target_type")
    set_default("client")
    set_showmenu(true)
    set_values("client", "server")
    set_description("BedrockUILib is client-only; server is rejected on purpose")
option_end()

option("tests")
    set_default(false)
    set_showmenu(true)
    set_description("Enable tests")
option_end()

-- Version is only set for releases: the release workflow exports BUL_VERSION from
-- the git tag. Every other build reports 0.0.0, by design. The description sandbox
-- cannot read files or run commands, so the environment variable is the only
-- channel available here; the header is rendered in before_build below.
local bul_project_version = os.getenv("BUL_VERSION") or "0.0.0"
bul_project_version = bul_project_version:gsub("^v", "")
set_version(bul_project_version)
-- Hard failure, not a degraded build. The library draws through the client UI
-- stack and its public API has no server meaning, so a server build must not
-- exist at all. The check runs in on_config: that is the scope where xmake
-- exposes raise(); the project scope sandbox has no abort primitive.

-- The client targets LeviLamina 26.60.x (Bedrock 1.26.60.x). Pinned to an exact
-- version on purpose: xmake builds LeviLamina from its git source, so a wildcard
-- makes it clone the newest patch from GitHub on every fresh machine.
add_requires("levilamina 26.60.*", {configs = {target_type = "client"}})

add_requires("levibuildscript")

if has_config("tests") then
    add_requires("gtest")
end

if not has_config("vs_runtime") then
    set_runtimes("MD")
end

local common_cxflags = {
    "/EHa",
    "/utf-8",
    "/W4",
    "/w44265",
    "/w44289",
    "/w44296",
    "/w45263",
    "/w44738",
    "/w45204",
}

local common_clang_flags = {
    "/EHs",
    "-Wno-microsoft-cast",
    "-Wno-invalid-offsetof",
    "-Wno-c++2b-extensions",
    "-Wno-microsoft-include",
    "-Wno-overloaded-virtual",
    "-Wno-ignored-qualifiers",
    "-Wno-missing-field-initializers",
    "-Wno-potentially-evaluated-expression",
    "-Wno-pragma-system-header-outside-header",
    {tools = {"clang_cl"}},
}

target("BedrockUILib")
    add_rules("@levibuildscript/linkrule")
    add_rules("@levibuildscript/modpacker")

    on_config(function (target)
        if (get_config("target_type") or "client") ~= "client" then
            raise("BedrockUILib is a client-only library and cannot be built for a server. "
                .. "Configure it with: xmake f -m release --target_type=client -y")
        end
    end)
    if is_plat("windows") then
        add_defines("NOMINMAX", "UNICODE", "_HAS_CXX23=1")
        set_exceptions("none") -- To avoid conflicts with /EHa.
        add_cxflags(common_cxflags)
        add_cxflags(common_clang_flags)
        set_toolchains("clang-cl")
        -- The clipboard, the modifier key state and the file dialog are plain
        -- Win32; no engine API exposes any of them.
        add_syslinks("user32", "comdlg32")
    end
    add_packages("levilamina")
    set_kind("shared")
    set_languages("c++20")
    set_symbols("debug")
    add_defines("BUL_TARGET_CLIENT")

    -- src/        pure logic: no mc/ or ll/ includes, unit-tested by the test target.
    -- src-client/ engine adapters: render context, input routing, screen host, mod entry.
    -- generated first: build/generated/bul/Version.h overrides src/bul/Version.h
    add_includedirs("build/generated")
    add_includedirs("src")
    add_includedirs("src-client")
    add_headerfiles("src/**.h", "src-client/**.h")
    -- src/ starts header-only; a plain glob would warn until the first .cpp exists.
    if #os.files("src/**.cpp") > 0 then
        add_files("src/**.cpp")
    end
    add_files("src-client/**.cpp")

    -- The demo panel exists only in --tests=y builds: it must never ship.
    if has_config("tests") then
        add_defines("BUL_DEMO")
        add_includedirs("src-demo")
        add_headerfiles("src-demo/**.h")
        add_files("src-demo/**.cpp")
    end

    -- Version header. This code has to live inside the build callback: the project
    -- description sandbox has no io library, and a closure created there keeps that
    -- restricted environment even when it runs later. Releases export BUL_VERSION
    -- from the tag; every other build renders 0.0.0. The header lands in
    -- build/generated (first on the include path); the checked-in
    -- src/bul/Version.h is only a fallback for an unbuilt checkout.
    before_build(function (target)
        local version = (os.getenv("BUL_VERSION") or "0.0.0"):gsub("^v", "")

        local major, minor, patch = version:match("^(%d+)%.(%d+)%.(%d+)$")
        if not major then
            major, minor, patch = "0", "0", "0"
        end

        local templatePath = path.join(os.projectdir(), "src", "bul", "Version.h.in")
        if not os.isfile(templatePath) then
            return
        end
        local header = io.readfile(templatePath)
        header = header:gsub("@BUL_VERSION_MAJOR@", major)
        header = header:gsub("@BUL_VERSION_MINOR@", minor)
        header = header:gsub("@BUL_VERSION_PATCH@", patch)
        header = header:gsub("@BUL_VERSION_STRING@", version)

        local outdir = path.join(os.projectdir(), "build", "generated", "bul")
        os.mkdir(outdir)
        local outfile = path.join(outdir, "Version.h")
        local previous = os.isfile(outfile) and io.readfile(outfile)
        if previous ~= header then
            io.writefile(outfile, header)
            print("version header generated for %s", version)
        end
    end)

    -- lang/<locale>.json is a runtime file: the modpacker rule copies the dll,
    -- pdb and manifest.json only, so the folder is copied next to the dll here.
    after_build(function (target)
        local langdir = path.join(os.projectdir(), "lang")
        if os.isdir(langdir) then
            local outdir = path.join(os.projectdir(), "bin", target:name(), "lang")
            os.mkdir(outdir)
            os.cp(path.join(langdir, "*.json"), outdir)
            cprint("${bright green}[Mod Packer]: ${reset}language files -> " .. outdir)
        end
    end)

-- Tests (LeviLamina layout: src-test/common, gtest, --tests=y):
--   xmake f -y -p windows -a x64 -m debug --tests=y --target_type=client
--   xmake build BedrockUILibTests && xmake run BedrockUILibTests
-- Kept out of the mod target: gtest supplies its own main() and a test run must
-- not need a running game.
if has_config("tests") then
    target("BedrockUILibTests")
        set_kind("binary")
        set_default(false)
        set_languages("c++20")
        set_symbols("debug")
        add_packages("gtest", "levilamina")
        add_defines("LL_DEBUG", "BUL_TARGET_CLIENT")
        add_includedirs("build/generated")
        add_includedirs("src")
        add_includedirs("src-client")
        add_includedirs("src-test/common")
        if is_plat("windows") then
            add_defines("NOMINMAX", "UNICODE")
            set_exceptions("none")
            add_cxflags(common_cxflags)
            add_cxflags(common_clang_flags)
            set_toolchains("clang-cl")
        end

        -- Every public header is force-compiled by include_all.cpp, so a header
        -- that stops compiling fails the build even when no test includes it.
        before_build(function ()
            local headers = {}
            for _, x in ipairs(os.files("src/**.h")) do
                table.insert(headers, "#include \"" .. path.relative(x, "src/") .. "\"")
            end
            for _, x in ipairs(os.files("src-client/**.h")) do
                table.insert(headers, "#include \"" .. path.relative(x, "src-client/") .. "\"")
            end
            io.writefile("src-test/common/include_all.cpp", table.concat(headers, "\n") .. "\n")
        end)

        after_build(function ()
            io.writefile("src-test/common/include_all.cpp", "// auto gen when build test\n")
        end)

        -- Pure model sources (src/) are compiled into the test binary; the
        -- engine adapters in src-client/ are only made available as headers.
        add_files("src/**.cpp")
        add_files("src-test/common/**.cpp")
end
