-- premake5.lua

-- you can redefine dependencies
local SDL2_DIR = os.getenv("SDL2_DIR") or "dependencies/SDL2"
local GLEW_DIR = os.getenv("GLEW_DIR") or "dependencies/glew"
local OPENAL_DIR = os.getenv("OPENAL_DIR") or "dependencies/openal-soft"
local REGION = os.getenv("REGION") or "NTSC_VERSION" -- or PAL_VERSION

if not (REGION == "NTSC_VERSION" or REGION == "PAL_VERSION") then
    error("'REGION' should be 'NTSC_VERSION' or 'PAL_VERSION'")
end

workspace "REDRIVER2"
    configurations { "Debug", "Release" }

    defines { VERSION } 

    filter "system:Windows"
        defines { "USE_32_BIT_ADDR", "PGXP" }

-- EMULATOR layer
project "PSX"
    kind "StaticLib"
    language "C++"
    compileas "C++"
    targetdir "bin/%{cfg.buildcfg}"

    includedirs { 
        ".", 
        "EMULATOR"
    }

    files { 
        "EMULATOR/**.h", 
        "EMULATOR/**.c", 
        "EMULATOR/**.cpp",
    }

    defines { "OGL", "GLEW" }

    includedirs { 
        SDL2_DIR.."/include",
        GLEW_DIR.."/include",
        OPENAL_DIR.."/include",
    }

    filter "system:Windows"
        links { 
            "opengl32",
            "glew32", 
            "SDL2", 
            "OpenAL32"
        }
    
        libdirs { 
            SDL2_DIR.."/lib/x86",
            GLEW_DIR.."/lib/Release/Win32",
            OPENAL_DIR.."/libs/Win32",
        }

-- game iteslf
project "REDRIVER2"
    kind "ConsoleApp"
    language "C++"
    compileas "C++"
    targetdir "bin/%{cfg.buildcfg}"

    includedirs { 
        ".", 
        "EMULATOR"
    }

    files { 
        "GAME/**.h", 
        "GAME/**.c", 
        "redriver2_psxpc.c",
        "DebugOverlay.cpp",
        --"THISDUST.C",
        "THISDUST.H",
    }

    filter "system:Windows"
        dependson { "PSX" }
        files { 
            "Windows/resource.h", 
            "Windows/Resource.rc", 
            "Windows/main.ico" 
        }

        defines { "OGL", "GLEW" }
    
        includedirs { 
            SDL2_DIR.."/include",
            GLEW_DIR.."/include",
            OPENAL_DIR.."/include",
        }

        links { "PSX" } -- only need to link emulator
    
        linkoptions {
			"/SAFESEH:NO", -- Image Has Safe Exception Handers: No. Because of openal-soft
		}

    filter "configurations:Debug"
        defines { 
            "DEBUG", 
            "DEBUG_OPTIONS",
            "COLLISION_DEBUG" 
         }
        symbols "On"

    filter "configurations:Release"
        defines { 
            "NDEBUG", 
            "DEBUG_OPTIONS",
            --"COLLISION_DEBUG" 
        }
        optimize "On"