target("TypeDuckDeployer")
  set_kind("binary")
  add_files("./*.cpp")
  add_rules("add_rcfiles", "use_weaselconstants", "subwin")
  add_links("imm32", "kernel32", "rime")
  add_deps("TypeDuckIPC", "RimeWithTypeDuck")
  add_files("$(projectdir)/PerMonitorHighDPIAware.manifest")
  add_ldflags("/DEBUG /OPT:ICF /LARGEADDRESSAWARE /ERRORREPORT:QUEUE")
  before_build(function(target)
    local target_dir = path.join(target:targetdir(), target:name())
    if not os.exists(target_dir) then
      os.mkdir(target_dir)
    end
    target:set("targetdir", target_dir)
  end)
  after_build(function(target)
    if is_arch("x86") then
      os.cp(path.join(target:targetdir(), "TypeDuckDeployer.exe"), "$(projectdir)/output/Win32")
      os.cp(path.join(target:targetdir(), "TypeDuckDeployer.pdb"), "$(projectdir)/output/Win32")
    else
      os.cp(path.join(target:targetdir(), "TypeDuckDeployer.exe"), "$(projectdir)/output")
      os.cp(path.join(target:targetdir(), "TypeDuckDeployer.pdb"), "$(projectdir)/output")
    end
  end)
