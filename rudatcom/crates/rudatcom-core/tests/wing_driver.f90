! Instrumentation only: the temporary original source calls these hooks to
! record wing-alone COMMON-block state after MAIN01's M29O35/M17O21 (lateral
! derivatives), at subsonic MAIN00 points only (MAIN03 also reuses MAIN01),
! and after MAIN04's M23O27 at supersonic points.
module rudatcom_wing_state
  implicit none
  logical :: subsonic = .false.
end module rudatcom_wing_state

! Called from CHECK once per input case.
subroutine rudatcom_case_probe
  implicit none
  open(unit=99, file='wing.txt', status='unknown', position='append')
  write(99, '(A)') 'CASE'
  close(99)
end subroutine rudatcom_case_probe

! Called by MAIN00 around its subsonic CALL MAIN01.
subroutine rudatcom_subsonic(flag)
  use rudatcom_wing_state
  implicit none
  logical, intent(in) :: flag
  subsonic = flag
end subroutine rudatcom_subsonic

! WING(1:200) coefficients plus the persistent work arrays A, B, WINGIN and
! the per-point C, D and STB.
subroutine rudatcom_wing_probe
  use rudatcom_wing_state
  implicit none
  real :: flc(160), pwing, wing(400), a(195), b(49), wingin(101)
  real :: c(51), d(55), cht(51), dht(55), dvt(55)
  real :: stb(135), tra(108), trah(108), stbh(135)
  integer :: nlog, nmach, i, nalpha, ig, nf, lf, k
  common /flgtcd/ flc
  common /iwing/ pwing, wing
  common /wingd/ a, b
  common /wingi/ wingin
  common /whaero/ c, d, cht, dht, dvt
  common /sbeta/ stb, tra, trah, stbh
  common /overly/ nlog, nmach, i, nalpha, ig, nf, lf, k
  if (.not. subsonic) return
  open(unit=99, file='wing.txt', status='unknown', position='append')
  write(99, '(A,*(1X,ES24.16E3))') 'POINT', flc(i+2), flc(i+42)
  write(99, '(A,*(1X,ES24.16E3))') 'WING', wing(1:200)
  write(99, '(A,*(1X,ES24.16E3))') 'A', a
  write(99, '(A,*(1X,ES24.16E3))') 'B', b
  write(99, '(A,*(1X,ES24.16E3))') 'C', c
  write(99, '(A,*(1X,ES24.16E3))') 'D', d
  write(99, '(A,*(1X,ES24.16E3))') 'STB', stb
  write(99, '(A,*(1X,ES24.16E3))') 'WINGIN', wingin(1:91)
  close(99)
end subroutine rudatcom_wing_probe

! Called at the end of MAIN04's lateral pass (supersonic points; NF < 0 is
! MAIN03's transonic pre-pass, skipped): the same rows plus SLG(1:141).
subroutine rudatcom_supersonic_probe
  implicit none
  real :: flc(160), pwing, wing(400), a(195), b(49), wingin(101)
  real :: c(51), d(55), cht(51), dht(55), dvt(55)
  real :: stb(135), tra(108), trah(108), stbh(135), gr(303)
  integer :: nlog, nmach, i, nalpha, ig, nf, lf, k
  common /flgtcd/ flc
  common /iwing/ pwing, wing
  common /wingd/ a, b
  common /wingi/ wingin
  common /whaero/ c, d, cht, dht, dvt
  common /sbeta/ stb, tra, trah, stbh
  common /supwh/ gr
  common /overly/ nlog, nmach, i, nalpha, ig, nf, lf, k
  open(unit=99, file='wing.txt', status='unknown', position='append')
  write(99, '(A,*(1X,ES24.16E3))') 'POINT', flc(i+2), flc(i+42)
  write(99, '(A,*(1X,ES24.16E3))') 'WING', wing(1:200)
  write(99, '(A,*(1X,ES24.16E3))') 'A', a
  write(99, '(A,*(1X,ES24.16E3))') 'B', b
  write(99, '(A,*(1X,ES24.16E3))') 'C', c
  write(99, '(A,*(1X,ES24.16E3))') 'D', d
  write(99, '(A,*(1X,ES24.16E3))') 'STB', stb
  write(99, '(A,*(1X,ES24.16E3))') 'WINGIN', wingin(1:91)
  write(99, '(A,*(1X,ES24.16E3))') 'SLG', gr(1:141)
  close(99)
end subroutine rudatcom_supersonic_probe
