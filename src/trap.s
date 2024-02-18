# 1 "src/trap.S"
# 1 "<built-in>"
# 1 "<command-line>"
# 31 "<command-line>"
# 1 "/usr/riscv64-linux-gnu/include/stdc-predef.h" 1 3
# 32 "<command-line>" 2
# 1 "src/trap.S"
.extern handleSupervisorTrap
.extern switchToSsp
.extern switchToSp
.global supervisorTrap
.align 4
supervisorTrap:
    # Nakon povratka iz switchToSsp f-je, nece biti iste vrednosti kao pre ulaska u ovu f-ju za sledece registre: ra, fp (i sp naravno).
    # Pa cemo ra i fp na korisnickom steku cuvati na ulasku u supervisorTrap, i odatle ih restaurirati pre izlaska iz supervisorTrap.
    # Sve ostalo, na sistemskom steku.
    addi sp, sp, -16 # ne moze samo za 8, mora pomeraj deljiv sa 16
    sd ra, 0*8(sp)
    sd fp, 1*8(sp)

    call switchToSsp
    mv fp, sp # Bez ovoga bi fp vrednost bila 0x1010..01, pa ne bismo mogli u handleru povratnu vrednost a0 da upisemo preko sacuvane lokacije 10*8(sp)

    addi sp, sp, -256
    .irp index,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31
    sd x\index, \index * 8(sp)
    .endr

    mv fp, sp # Dok nije bilo sistemskih stekova ovde je bilo neophodno. Vise nije, ali nek ostane za slucaj da izbacim sist stekove.
    call handleSupervisorTrap

    .irp index,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31
    ld x\index, \index * 8(sp)
    .endr
    addi sp, sp, 256

    call switchToSp

    ld ra, 0*8(sp)
    ld fp, 1*8(sp)
    addi sp, sp, 16
sret
