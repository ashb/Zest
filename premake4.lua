solution "Zest"

  targetdir "build"

  configurations { "Release", "Debug" }

  flags { "ExtraWarnings" }
  
  configuration "Release"
      flags { "Optimize" }
  configuration "Debug"
      flags { "Symbols" }
 
  configuration {}

  -- Hack until i get pkg-config support in premake
  defines {"BOOST_PARAMETER_MAX_ARITY=10"}

  project "libzest"
    targetname "zest"
    --targetextension ".dylib"

    kind "SharedLib"
    language "C++"
    files { "src/*.cpp", "src/*.hpp" }

    links { "flusspferd" }

    configuration { "windows" }
      defines {"_WIN32_WINDOWS"}
      includedirs { 
        "C:/progra~1/flusspferd/include",
        "C:/spidermonkey/include",
      }

      libdirs {
        "C:/progra~1/flusspferd/lib",
      }
      links {"ws2_32", "mswsock"}

