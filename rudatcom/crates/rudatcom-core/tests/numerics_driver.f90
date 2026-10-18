! Live probe of unmodified original routines, compiled by numerics.rs.
! Nothing numerical is reimplemented here; stdin selects a routine and supplies
! its arguments. Scratch units isolate the legacy diagnostic files.
program rudatcom_numerics_probe
  use iso_fortran_env, only: output_unit
  implicit none
  real :: pi, deg, unused, rad
  integer :: kand
  common /consnt/ pi, deg, unused, rad, kand
  integer :: nn(5), nf, nm(2), iovly
  common /overly/ nn, nf, nm, iovly
  logical :: flog(21)
  common /flolog/ flog
  real :: aht(195), htin(131), dw(35), tcd(58)
  common /htdata/ aht
  common /hti/ htin
  common /supdw/ dw, tcd
  integer :: op, ios, n, mode, i, k, d, lo(4), hi(4), lens(4), count
  integer :: dummy_in, mi, ng, mess(20), rout(2), ig
  real :: x(16,4), y(4096), at(4), a(16), rhs(4), out(64), c(6)
  real :: step, sweep, efi, efo, boch(4), gd1(14), gd2(14), gd3(14)
  real :: tan_te, tan_le, semispan, chord, fraction
  logical :: ascending, no_interpolation
  pi = 3.141592654
  deg = 0.01745329
  unused = 1.e-30
  rad = 57.2957795
  kand = 0
  nn = 0
  nf = 0
  nm = 0
  iovly = 0
  flog = .false.
  aht = 0.
  htin = 0.
  dw = 0.
  tcd = 0.
  dummy_in = 0
  mi = 0
  ng = 0
  c = 0.
  mess = 0
  rout = 0
  open(unit=12, status='scratch', form='formatted')
  do
    read(*, *, iostat=ios) op
    if (ios < 0) exit
    if (ios /= 0) stop 1
    select case(op)
    case(1)
      read(*, *) x(1:3,1), y(1:3), at(1)
      out(1) = 0.
      call quad(x, y, at(1), out(1))
      out(2) = transfer('DERI', 0.)
      call quad(x, y, at(1), out(2))
      k = 2
    case(2)
      read(*, *) n, step
      read(*, *) y(1:n)
      call quadin(y, n, step, out(1))
      k = 1
    case(3)
      read(*, *) n, mode
      read(*, *) x(1:n,1), y(1:n)
      call trapz(y, n, x, out, mode)
      k = n
      if (mode == 1 .or. mode < 0) k = 1
    case(4)
      read(*, *) a, rhs
      call det4(a, out(1))
      call simul4(a, rhs, out(2))
      k = 5
    case(5)
      read(*, *) boch, sweep
      call agenr(boch, sweep, out)
      k = 16
    case(6)
      read(*, *) n, lo(1), hi(1)
      read(*, *) x(1:n,1), y(1:n), at(1)
      call tbfunx(at(1), out(1), out(2), n, x, y, c, dummy_in, mi, ng, &
                   lo(1), hi(1), mess, 1, rout)
      k = 2
    case(7)
      read(*, *) d, mode
      read(*, *) lens(1:d), lo(1:d), hi(1:d), at(1:d)
      do i=1,d
        read(*, *) x(1:lens(i),i)
      end do
      count = product(lens(1:d))
      read(*, *) y(1:count)
      if (mode == 1) then
        call interx(d, x, at, lens, y, out(1), 16, count, lo(1), lo(2), &
                     lo(3), lo(4), hi(1), hi(2), hi(3), hi(4), mess, 1, rout)
      else
        select case(d)
        case(1)
          call tlin1x(x, y, lens(1), at(1), out(1), lo(1), hi(1), mess, 1, rout)
        case(2)
          call tlinex(x(1,1), x(1,2), y, lens(1), lens(2), at(1), at(2), &
                      out(1), lo(1), lo(2), hi(1), hi(2), mess, 1, rout)
        case(3)
          call tlin3x(x(1,1), x(1,2), x(1,3), y, lens(1), lens(2), lens(3), &
                      at(1), at(2), at(3), out(1), lo(1), lo(2), lo(3), &
                      hi(1), hi(2), hi(3), mess, 1, rout)
        case(4)
          call tlin4x(x(1,1), x(1,2), x(1,3), x(1,4), y, lens(1), lens(2), &
                      lens(3), lens(4), at(1), at(2), at(3), at(4), out(1), &
                      lo(1), lo(2), lo(3), lo(4), hi(1), hi(2), hi(3), hi(4), mess, 1, rout)
        end select
      end if
      k = 1
    case(8)
      read(*, *) boch, sweep, efi, efo
      flog(21) = .false.
      call gdelta(gd1, gd2, gd3, efi, efo, boch, sweep)
      out(1:14) = gd1
      out(15:28) = gd2
      out(29:42) = gd3
      k = 42
    case(9)
      read(*, *) sweep, tan_te, tan_le, semispan, chord
      flog(21) = .true.
      aht(80) = tan_te
      aht(62) = tan_le
      aht(10) = chord
      htin(3) = semispan
      call gdelta(gd1, gd2, gd3, efi, efo, boch, sweep)
      out(1:4) = tcd(43:46)
      k = 4
    case(10)
      read(*, *) n
      read(*, *) x(1:n,1), at(1)
      ascending = x(1,1) <= x(n,1)
      no_interpolation = .false.
      fraction = -1.
      call glook(n, at(1), x, ascending, no_interpolation, ig, fraction)
      out(1) = real(ig - 1)
      out(2) = fraction
      k = 2
    case default
      stop 2
    end select
    write(output_unit, '(*(ES24.16E3,1X))') out(1:k)
  end do
end program
