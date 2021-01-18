addi    $a0, $zero, 1
addi	$a1, $zero, 2
addi 	$a2, $zero, 2
!skplt   $a0, $a2
!addi 	$t0, $zero, 1
skple   $a1, $a2
addi 	$t0, $zero, 1
!skpeq   $a1, $a1
!addi 	$t0, $zero, 1
!skpne   $a2, $a0
!addi 	$t0, $zero, 1
!skpgt   $a2, $a0
!addi 	$t0, $zero, 1
!skpge   $a2, $a0
!addi 	$t0, $zero, 1
halt