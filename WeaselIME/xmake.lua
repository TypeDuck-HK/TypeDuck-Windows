target("TypeDuckIME")
  set_kind("shared")
  add_files("./*.cpp")
  add_rules("add_rcfiles")
  add_links("advapi32", "imm32", "user32", "gdi32")
  add_deps("TypeDuckIPC", "TypeDuckUI")
  local fname = ''

  before_build(function(target)
    local target_dir = path.join(target:targetdir(), target:name())
    if not os.exists(target_dir) then
      os.mkdir(target_dir)
    end
    target:set("targetdir", target_dir)
  end)

  after_build(function(target)
    os.cp(path.join(target:targetdir(), "*.ime"), "$(projectdir)/output")
  end)
  if is_arch("x86") then
    fname = "TypeDuck.ime"
  elseif is_arch("x64") then
    fname = "TypeDuckx64.ime"
  elseif is_arch("arm") then
    fname = "TypeDuckARM.ime"
  elseif is_arch("arm64") then
    fname = "TypeDuckARM64.ime"
  end
  set_filename(fname)
