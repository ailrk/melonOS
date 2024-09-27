from lldb import SBBreakpoint, SBBreakpointLocation, SBDebugger, SBFrame, SBStructuredData, SBTarget, debugger
import lldb


def trap_bp(frame: SBFrame,
       bp_loc: SBBreakpointLocation,
       extra_args: SBStructuredData,
       internal_dict: dict):
    assert(frame.GetFunctionName() == "trap")
    print(frame.GetSymbol())


# Trapframe breakpoint
def add_trap(target: SBTarget):
    b: SBBreakpoint = target.BreakpointCreateByName("trap")
    b.SetScriptCallbackFunction("debug.trap_bp")


def __lldb_init_module(debugger: SBDebugger, internal):
    target: SBTarget = debugger.GetSelectedTarget()
    print("Melonos Debugging scripts")
    add_trap(target)
