! Instrumentation only: snapshot original COMMON blocks immediately after CONV,
! then invoke the original M02O02, EQSPCE, GETMAX and BODYJM geometry calculations.
! The temporary source returns from BODYJM before its aerodynamic calculations.
subroutine rudatcom_geometry_probe
  implicit none
  real :: opts(4), syn(19), bd(762), wi(101), hi(154), vi(316), bi(129), flc(160)
  real :: a(195), b(49), ah(195), bh(49), av(195), af(195), power(50), flap(138)
  logical :: flags(34)
  common /option/ opts
  common /syntss/ syn
  common /bdata/ bd
  common /wingi/ wi
  common /hti/ hi
  common /vti/ vi
  common /bodyi/ bi
  common /flgtcd/ flc
  common /wingd/ a, b
  common /htdata/ ah, bh
  common /vtdata/ av, af
  common /flolog/ flags
  common /power/ power
  common /flapin/ flap
  real :: xe(20), re(20), pe(20), se(20), ds(20), xmax, smax
  integer :: nx, imax, k
  ! xxIN slots M50O62/SECO can fill; 95-100 are the wing-only CSLOPE slopes
  ! (HTIN/VTIN 95+ hold the planform RLPH/SVWB arrays instead).
  integer, parameter :: slots(76) = [10, (k, k = 16, 69), (k, k = 72, 91), 93]
  open(unit=99, file='geometry.txt', status='unknown', position='append')
  write(99, '(A)') 'CASE'
  write(99, '(A,*(1X,ES24.16E3))') 'SYN', syn(1:9), syn(11:19)
  write(99, '(A,*(1X,ES24.16E3))') 'FLC', flc(43:62), flc(97:116), &
      flc(74:93), flc(117:136), flc(137:157)
  write(99, '(A,*(1X,ES24.16E3))') 'TVT', vi(155:162)
  write(99, '(A,*(1X,ES24.16E3))') 'GROUND', flc(64:73)
  write(99, '(A,*(1X,ES24.16E3))') 'POWER', power
  write(99, '(A,*(1X,ES24.16E3))') 'FLAP', flap
  if (flags(3)) then
    write(99, '(A,*(1X,ES24.16E3))') 'BODY', bi(1:126), bi(129)
  end if
  call m50o62
  if (flags(4)) write(99, '(A,*(1X,ES24.16E3))') 'WSCHR', sections(wi(slots)), wi(95:100)
  if (flags(7)) write(99, '(A,*(1X,ES24.16E3))') 'HSCHR', sections(hi(slots))
  if (flags(9)) write(99, '(A,*(1X,ES24.16E3))') 'VSCHR', sections(vi(slots))
  if (flags(31)) write(99, '(A,*(1X,ES24.16E3))') 'FSCHR', sections(vi(slots + 162))
  call m02o02
  write(99, '(A,*(1X,ES24.16E3))') 'POSTSYN', syn(1:9), syn(11:19)
  write(99, '(A,*(1X,ES24.16E3))') 'REF', opts
  if (flags(4)) write(99, '(A,*(1X,ES24.16E3))') 'WG', a
  if (flags(7)) write(99, '(A,*(1X,ES24.16E3))') 'HT', ah
  if (flags(9)) write(99, '(A,*(1X,ES24.16E3))') 'VT', av
  if (flags(31)) write(99, '(A,*(1X,ES24.16E3))') 'VF', af
  if (flags(4)) write(99, '(A,*(1X,ES24.16E3))') 'SYNGEOM', bd(66:68), bd(78:80), bd(87)
  if (flags(3)) then
    nx = bi(1) + 0.5
    call eqspce(bi(2), bi(62), bi(42), bi(22), nx, xe, re, pe, se, 20, ds)
    write(99, '(A,*(1X,ES24.16E3))') 'EQUAL', xe, re, pe, se, ds
    call getmax(bi(2), bi(22), nx, xmax, smax, imax)
    call bodyjm
    write(99, '(A,*(1X,ES24.16E3))') 'BODYSTATS', bi(nx+1), bi(nx+21), smax, &
        real(imax-1), bd(34), bd(275), bd(35), bd(536:535+nx)
  end if
  close(99)
contains
  ! SECO stores the LOGICAL CAMBER (slot 64, element 50) into a REAL slot;
  ! report it as rudatcom's 1.0/0.0 logical encoding (UNUSED stays as is).
  function sections(raw) result(out)
    real, intent(in) :: raw(76)
    real :: out(76)
    out = raw
    if (transfer(raw(50), 0) == 1) out(50) = 1.0
    if (transfer(raw(50), 0) == 0) out(50) = 0.0
  end function
end subroutine
