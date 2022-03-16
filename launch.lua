---__init__
if _G["__init__"] then
    local arg = ...
    return {
        thread = 16,
        enable_console = true,
        logfile = string.format("log/moon-%s-%s.log", arg[1], os.date("%Y-%m-%d-%H-%M-%S")),
        loglevel = "DEBUG",
    }
end
