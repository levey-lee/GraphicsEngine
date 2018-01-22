
local projectTitle = "DiamondGraphics"

if _ACTION == "clean" then
  os.rmdir("../bin")
  os.rmdir("./projects")
  os.rmdir("./ipch")
  os.rmdir(".vs")
  os.rmdir("Debug")
  os.remove("Diamond.sln")
  os.remove("Diamond.sdf")
  os.remove("Diamond.suo")
  os.remove("Diamond.v11.suo")
  os.remove("Diamond.v12.suo")
  os.remove("*.DB")
  os.exit()
end

solution "Diamond"
  configurations { "Debug", "ReleaseSymbols", "Release" }
  project(projectTitle)
    targetname(projectTitle:lower())
    kind "ConsoleApp"
    language "C++"
    location "projects"
    pchsource "../src/Precompiled.cpp"
    pchheader "Precompiled.h"
    includedirs { "../inc", "../dep" }
    libdirs { "../dep/FreeGLUT", "../dep/GLEW", "../dep/STB" , "../dep/AntTweakBar"}
    links { "freeglut", "glew32" , "AntTweakBar"}
    files { "../inc/**.h", "../src/**.cpp" }
    configuration "Debug"
      targetdir "../bin/debug"
      defines { "_DEBUG", "ASSET_PATH=\"../../assets/\"" }
      symbols "On"
      links {"AntTweakBar"}
      postbuildcommands {
        "copy ..\\..\\dep\\GLEW\\glew32.dll ..\\..\\bin\\debug\\",
        "copy ..\\..\\dep\\AntTweakBar\\AntTweakBar.dll ..\\..\\bin\\debug\\",
        "copy ..\\..\\dep\\FreeGLUT\\freeglut.dll ..\\..\\bin\\debug\\" }
    configuration "ReleaseSymbols"
      targetdir "../bin/release"
      defines { "NDEBUG", "ASSET_PATH=\"../../assets/\"" }
      optimize "On"
      symbols "On"
      links {"AntTweakBar"}
      linkoptions { "/LTCG" } -- allows whole-program optimization
      postbuildcommands {
        "copy ..\\..\\dep\\GLEW\\glew32.dll ..\\..\\bin\\release\\",
        "copy ..\\..\\dep\\AntTweakBar\\AntTweakBar.dll ..\\..\\bin\\release\\",
        "copy ..\\..\\dep\\FreeGLUT\\freeglut.dll ..\\..\\bin\\release\\" }
    configuration "Release"
      targetdir "../bin/release"
      defines { "NDEBUG", "ASSET_PATH=\"../../assets/\"" }
      optimize "On"
      links {"AntTweakBar"}
      linkoptions { "/LTCG" } -- allows whole-program optimization
      postbuildcommands {
        "copy ..\\..\\dep\\GLEW\\glew32.dll ..\\..\\bin\\release\\",
        "copy ..\\..\\dep\\AntTweakBar\\AntTweakBar.dll ..\\..\\bin\\release\\",
        "copy ..\\..\\dep\\FreeGLUT\\freeglut.dll ..\\..\\bin\\release\\" }
