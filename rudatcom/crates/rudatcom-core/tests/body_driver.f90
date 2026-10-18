! Instrumentation only: the temporary original source calls these hooks to
! record body-alone results after MAIN01's M04O04/M06O06 overlay, for
! subsonic MAIN00 points only (MAIN03 also reuses MAIN01 for transonic work).
module rudatcom_body_state
  implicit none
  logical :: subsonic = .false.
end module rudatcom_body_state

! Called from CHECK once per input case.
subroutine rudatcom_case_probe
  implicit none
  open(unit=99, file='body.txt', status='unknown', position='append')
  write(99, '(A)') 'CASE'
  close(99)
end subroutine rudatcom_case_probe

! Called by MAIN00 around its subsonic CALL MAIN01.
subroutine rudatcom_subsonic(flag)
  use rudatcom_body_state
  implicit none
  logical, intent(in) :: flag
  subsonic = flag
end subroutine rudatcom_subsonic

subroutine rudatcom_body_probe
  use rudatcom_body_state
  implicit none
  real :: bd(762), flc(160), pbody, body(400)
  integer :: nlog, nmach, i, nalpha, ig, nf, lf, k
  common /bdata/ bd
  common /flgtcd/ flc
  common /ibody/ pbody, body
  common /overly/ nlog, nmach, i, nalpha, ig, nf, lf, k
  if (.not. subsonic) return
  open(unit=99, file='body.txt', status='unknown', position='append')
  write(99, '(A,*(1X,ES24.16E3))') 'POINT', flc(i+2), flc(i+42)
  write(99, '(A,*(1X,ES24.16E3))') 'COEF', body(1:200)
  write(99, '(A,*(1X,ES24.16E3))') 'DRAG', bd(1), bd(56), bd(57), bd(75), &
      bd(90), bd(91), bd(92), bd(93), bd(59), bd(60), bd(61)
  write(99, '(A,*(1X,ES24.16E3))') 'METHOD', bd(9), bd(76), bd(81), bd(62), &
      bd(135:154)
  close(99)
end subroutine rudatcom_body_probe
