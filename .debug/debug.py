from lldb import SBBreakpointLocation, SBCommandReturnObject, SBDebugger, SBFrame, SBModule, SBStructuredData, SBSymbol, SBTarget, SBValue
import argparse
import shlex

def describe_reg(reg: str, target: SBTarget, frame: SBFrame) -> SBSymbol | str:
    general = frame.GetRegisters()[0]
    r: int = general.GetChildMemberWithName(reg).GetValueAsUnsigned()
    module: SBModule = target.GetModuleAtIndex(0)
    for sec in module.section_iter():
        for sym in module.symbol_in_section_iter(sec):
            (start, end) = (sym.GetStartAddress().GetLoadAddress(target),
                            sym.GetEndAddress().GetLoadAddress(target))
            if r >= start and r < end: return f"{reg}:{r:x} {sym}"
    return f"{reg:x}:{r} points to unknown location"

def describe_esp(target, frame): return describe_reg("esp", target, frame)
def describe_eip(target, frame): return describe_reg("eip", target, frame)

def print_trapframe(tf: SBValue):
    print("Trapframe: %#x" % int(tf.GetChildMemberWithName("trapno").GetValue()))
    print("  edi:    %#x" % int(tf.GetChildMemberWithName("edi").GetValue()))
    print("  esi:    %#x" % int(tf.GetChildMemberWithName("esi").GetValue()))
    print("  ebp:    %#x" % int(tf.GetChildMemberWithName("ebp").GetValue()))
    print("  _esp:   %#x" % int(tf.GetChildMemberWithName("_esp").GetValue()))
    print("  ebx:    %#x" % int(tf.GetChildMemberWithName("ebx").GetValue()))
    print("  edx:    %#x" % int(tf.GetChildMemberWithName("edx").GetValue()))
    print("  ecx:    %#x" % int(tf.GetChildMemberWithName("ecx").GetValue()))
    print("  eax:    %#x" % int(tf.GetChildMemberWithName("eax").GetValue()))
    print("  gs:     %#x" % int(tf.GetChildMemberWithName("gs").GetValue()))
    print("  fs:     %#x" % int(tf.GetChildMemberWithName("fs").GetValue()))
    print("  es:     %#x" % int(tf.GetChildMemberWithName("es").GetValue()))
    print("  ds:     %#x" % int(tf.GetChildMemberWithName("ds").GetValue()))
    print("  trapno: %#x" % int(tf.GetChildMemberWithName("trapno").GetValue()))
    print("  err:    %#x" % int(tf.GetChildMemberWithName("err").GetValue()))
    print("  edi:    %#x" % int(tf.GetChildMemberWithName("eip").GetValue()))
    print("  cs:     %#x" % int(tf.GetChildMemberWithName("cs").GetValue()))
    print("  eflags: %s"  % bin(int(tf.GetChildMemberWithName("eflags").GetValue())))
    print("  esp:    %#x" % int(tf.GetChildMemberWithName("esp").GetValue()))
    print("  ss:     %#x" % int(tf.GetChildMemberWithName("ss").GetValue()))

def print_process(p: SBValue):
    file      = p.GetChildMemberWithName("file")
    def file_summary(fd: int, f: SBValue):
        nref: SBValue = f.GetChildMemberWithName("nref")
        t = f.GetChildMemberWithName("type")
        inode = f.GetChildMemberWithName("inode")
        return f"    file {fd}: {t}, {inode}, {nref}"
    file_ = "\n"
    file: SBValue
    for (fd, f) in enumerate(file.children):
        file_ += file_summary(fd, f)
        if (fd != len(file.children) - 1):
            file_ += "\n"
    print(f"Process %s" % p.GetChildMemberWithName("pid").GetValue())
    print(f"  size:      %s" % p.GetChildMemberWithName("size").GetValue())
    print(f"  pgdir:     %s" % p.GetChildMemberWithName("pgdir").GetValue())
    print(f"  kstack:    %s" % p.GetChildMemberWithName("kstack").GetValue())
    print(f"  parent:    %s" % p.GetChildMemberWithName("parent").GetValue())
    print(f"  state:     %s" % p.GetChildMemberWithName("state"))
    print(f"  trapframe: %s" % p.GetChildMemberWithName("trapframe").GetValue())
    print(f"  context:   %s" % p.GetChildMemberWithName("context").GetValue())
    print(f"  chan:      %s" % p.GetChildMemberWithName("chan").GetValue())
    print(f"  kill:      %s" % p.GetChildMemberWithName("kill").GetValue())
    print(f"  name:      %s" % p.GetChildMemberWithName("name").GetValue())
    print(f"  file:      %s" % file_)

# Commands

def mos_print_process(debugger: SBDebugger, command: str, result: SBCommandReturnObject, internal_dict: dict):
    parser = argparse.ArgumentParser(prog="mos_print_process", description="print melon os process")
    idx_group = parser.add_mutually_exclusive_group(required=True)
    idx_group.add_argument("-i", '--index', type=int, help="print process by index in ptable")
    idx_group.add_argument("-p", '--pid', type=int, help="print process by pid")
    args = parser.parse_args(shlex.split(command))
    target: SBTarget = debugger.GetSelectedTarget()
    ptable: SBValue = target.FindGlobalVariables("ptable", 1)[0]
    processes: SBValue = ptable.GetChildMemberWithName("t");
    if (pid := vars(args)['pid']) is not None:
        for p in processes:
            if p.GetChildMemberWithName("pid") == pid:
                print_process(p)
                return
        print(f"Can't find pid {pid}")
    elif (idx := vars(args)['index']) is not None:
        if (idx >= len(processes) or idx < 0):
            raise Exception(f"Impossible index {idx} on {len(processes)} processes")
        print_process(processes.GetChildAtIndex(idx))
        return
    else:
        raise Exception("Can't print process without index and pid")

def mos_esp(debugger: SBDebugger, command: str, result: SBCommandReturnObject, internal_dict: dict):
    print(describe_esp(target := debugger.GetSelectedTarget(),
                       target.GetProcess().GetSelectedThread().GetSelectedFrame()))

def mos_eip(debugger: SBDebugger, command: str, result: SBCommandReturnObject, internal_dict: dict):
    print(describe_eip(target := debugger.GetSelectedTarget(),
                       target.GetProcess().GetSelectedThread().GetSelectedFrame()))

def bp_trap(frame: SBFrame,
       bp_loc: SBBreakpointLocation,
       extra_args: SBStructuredData,
       internal_dict: dict):
    tfptr: SBValue = frame.FindVariable("tf");
    print_trapframe(tfptr.deref)

def bp_trap_add(debugger):
    debugger.HandleCommand("br set --name trap")
    debugger.HandleCommand("br com add -F debug.bp_trap")

def __lldb_init_module(debugger: SBDebugger, internal):
    print("Melonos Debugging scripts")
    # commands
    debugger.HandleCommand(f"com scr add -f debug.mos_print_process mos_print_process")
    debugger.HandleCommand(f"com scr add -f debug.mos_esp mos_esp")
    debugger.HandleCommand(f"com scr add -f debug.mos_eip mos_eip")
