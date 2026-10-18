! Instrumentation only: the temporary original source calls these hooks to
! record the tail COMMON-block state (HT, VT, VF, TVTPAN) after MAIN01's
! M29O35/M17O21 at subsonic MAIN00 points of cases without a body.
module rudatcom_tail_state
  implicit none
  logical :: subsonic = .false.
end module rudatcom_tail_state

! Called from CHECK once per input case.
subroutine rudatcom_case_probe
  implicit none
  open(unit=99, file='tail.txt', status='unknown', position='append')
  write(99, '(A)') 'CASE'
  close(99)
end subroutine rudatcom_case_probe

! Called by MAIN00 around its subsonic CALL MAIN01 (MAIN03 reuses MAIN01).
subroutine rudatcom_subsonic(flag)
  use rudatcom_tail_state
  implicit none
  logical, intent(in) :: flag
  subsonic = flag
end subroutine rudatcom_subsonic

! HT(1:200), VT(1:200), VF(1:200) plus their work arrays and both STB
! blocks (STB holds the VT section's terms, STBH the HT's or the VF's).
subroutine rudatcom_tail_probe
  use rudatcom_tail_state
  implicit none
  real :: flc(160), pht, ht(380), pvt, vt(380), pvf, vf(380)
  real :: aht(195), bht(49), htin(154), avt(195), avf(195)
  real :: vtin(154), tvtin(8), vfin(154)
  real :: c(51), d(55), cht(51), dht(55), dvt(55), dvf(55)
  real :: stb(135), tra(108), trah(108), stbh(135)
  integer :: nlog, nmach, i, nalpha, ig, nf, lf, k
  common /flgtcd/ flc
  common /iht/ pht, ht
  common /ivt/ pvt, vt
  common /ivf/ pvf, vf
  common /htdata/ aht, bht
  common /vtdata/ avt, avf
  common /hti/ htin
  common /vti/ vtin, tvtin, vfin
  common /whaero/ c, d, cht, dht, dvt, dvf
  common /sbeta/ stb, tra, trah, stbh
  common /overly/ nlog, nmach, i, nalpha, ig, nf, lf, k
  if (.not. subsonic) return
  open(unit=99, file='tail.txt', status='unknown', position='append')
  write(99, '(A,*(1X,ES24.16E3))') 'POINT', flc(i+2), flc(i+42)
  write(99, '(A,*(1X,ES24.16E3))') 'HT', ht(1:200)
  write(99, '(A,*(1X,ES24.16E3))') 'AHT', aht
  write(99, '(A,*(1X,ES24.16E3))') 'BHT', bht
  write(99, '(A,*(1X,ES24.16E3))') 'CHT', cht
  write(99, '(A,*(1X,ES24.16E3))') 'DHT', dht
  write(99, '(A,*(1X,ES24.16E3))') 'STBH', stbh
  write(99, '(A,*(1X,ES24.16E3))') 'HTIN', htin
  write(99, '(A,*(1X,ES24.16E3))') 'VT', vt(1:200)
  write(99, '(A,*(1X,ES24.16E3))') 'AVT', avt
  write(99, '(A,*(1X,ES24.16E3))') 'VTIN', vtin
  write(99, '(A,*(1X,ES24.16E3))') 'DVT', dvt
  write(99, '(A,*(1X,ES24.16E3))') 'VF', vf(1:200)
  write(99, '(A,*(1X,ES24.16E3))') 'AVF', avf
  write(99, '(A,*(1X,ES24.16E3))') 'VFIN', vfin
  write(99, '(A,*(1X,ES24.16E3))') 'DVF', dvf
  write(99, '(A,*(1X,ES24.16E3))') 'STB', stb
  close(99)
end subroutine rudatcom_tail_probe
