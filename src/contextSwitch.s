# 1 "src/contextSwitch.S"
# 1 "<built-in>"
# 1 "<command-line>"
# 31 "<command-line>"
# 1 "/usr/riscv64-linux-gnu/include/stdc-predef.h" 1 3
# 32 "<command-line>" 2
# 1 "src/contextSwitch.S"
.global _ZN3PCB13contextSwitchEPNS_7ContextES1_
_ZN3PCB13contextSwitchEPNS_7ContextES1_:
    sd ra, 0*8(a0)
# sd sp, 1*8(a0) # Ovako dok nije bilo sistemskih stekova (cuva se korisnicki stek stare niti).
    sd sp, 2*8(a0) # Ovako kad postoje sistemski stekovi (cuva se sistemski stek stare niti, a korisnicki se sacuvao kad smo usli u prek rut).

    ld ra, 0*8(a1)
# ld sp, 1*8(a1) # Ovako dok nije bilo sistemskih stekova.
    ld sp, 2*8(a1) # Ovako kad postoje sistemski stekovi.

    ret
