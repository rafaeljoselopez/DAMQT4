!  Copyright 2013-2021, Jaime Fernandez Rico, Rafael Lopez, Ignacio Ema,
!  Guillermo Ramirez, Anmol Kumar, Sachin D. Yeole, Shridhar R. Gadre
! 
!  This file is part of DAM320.
! 
!  DAM320 is free software: you can redistribute it and/or modify
!  it under the terms of the GNU General Public License as published by
!  the Free Software Foundation, either version 3 of the License, or
!  (at your option) any later version.
! 
!  DAM320 is distributed in the hope that it will be useful,
!  but WITHOUT ANY WARRANTY; without even the implied warranty of
!  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
!  GNU General Public License for more details.
! 
!  You should have received a copy of the GNU General Public License
!  along with DAM320.  If not, see <http://www.gnu.org/licenses/>.
!
!------------------------------------------------------------------------
!
! Program for tabulation of the electrostatic potential from the representation of the molecular density performed with
! DAM320
!
!
! Version of December 2025
!
MODULE SELFREPULSION
    USE DAM_400_D
    USE DAM_400_CONST_D
    IMPLICIT NONE
    integer(KINT), parameter :: mxsel = 200
    integer(KINT) :: iatomsel(mxsel)
    integer(KINT) :: nsel
END MODULE
  program self_repulsion
    USE DAM_400_D
    USE DAM_400_CONST_D
    USE DAM_400_DATA_D
    USE DAMPOT_400_D
    USE SELFREPULSION
    implicit none
    integer(KINT) :: i, ia, ierr, j, k, knt, kntlist, ltab, mtab, nlist, ncntab, ntab, ntabtot
    integer(KINT), parameter :: mxnlist = 100
    logical :: lrlist
    real(KREAL) :: dltr, r, rfin, rini
    real(KREAL4) :: tarray(2), tiempo, dtime
    real(KREAL) :: rlist(mxnlist)
    real(KREAL), allocatable :: rtabular(:)
    namelist / options / iatomsel, lmaxrep, ltab, mtab, lrlist, nlist, rlist, rini, rfin, dltr, iswindows, &
                         filename, longoutput, umbrlargo, ncntab
    tiempo = dtime(tarray)
!    Namelist default values
    iatomsel(1) = 1	    ! Atom selected for tabulation
    lmaxrep = 10            ! highest "l" in the expansion of the potential
    ltab = 0			! l  value for tabulation
    mtab = 0			! m  value for tabulation
    lrlist = .false.	! If true, list of r values for tabulation supplied in rlist
    nlist = 0			! Number of elements in rlist
    rini = cero		! Lower bound of interval for tabulation
    rfin = dos		! Upper bound of interval for tabulation
    dltr = ri(10)		! Step for tabulation
    iswindows = .false.        ! .true. if running on a MS-windows system
    filename = ""            ! root file name for .selfrp files
    longoutput = .false.    ! If .true. a more detailed output is given
    umbrlargo = 1.d-9        ! Threshold for determining the short-range radius
    ncntab = 1		! Number of atoms selected for tabulation (dummy, only used by DAMQT GUI)

!    End of namelist defaults

    read(5,OPTIONS)
    read(5,*) projectname
    if (longoutput) write(6,"(1x,'project name : ',a,/,1x,'==============')") projectname
    if (iswindows) then
        dirsep = "\\"
        i = index(projectname,dirsep,.true.)    ! Checks position of last directory name separator
        if (i .eq. 0) then    ! This is intended for MinGW, whose directory separator in windows is also /
                dirsep = "/"
                i = index(projectname,dirsep,.true.)    ! Checks position of last directory name separator
        endif
    else
        dirsep = "/"
        i = index(projectname,dirsep,.true.)    ! Checks position of last directory name separator
    end if

    if (len_trim(filename).eq.0) then
        filename = projectname
    else
        filename = projectname(1:i)//trim(filename)
    endif
 
    call consta        !    Computes auxiliary constants

    lgbsgz = .false.    ! Checks whether the .ggbs or .sgbs file is gzipped or not
    inquire(file=trim(projectname)//".ggbs.gz", exist=lgbsgz, iostat=ierr)
    if (ierr .eq. 0 .and. lgbsgz) then
        call system ("gunzip "//trim(projectname)//".ggbs.gz")
    else
        inquire(file=trim(projectname)//".sgbs.gz", exist=lgbsgz, iostat=ierr)
        if (ierr .eq. 0 .and. lgbsgz) then
            call system ("gunzip "//trim(projectname)//".sgbs.gz")
        endif
    endif
    
    if (dltr .le. cero) dltr = ri(10)
    ntab = nint((max(rini,rfin) - min(rini,rfin)) / dltr ) + 1
!     if (nlist .gt. mxnlist) then
!         write(6,"('Number of values of r supplied in rlist greather than maximum allowed.')")
!         write(6,"('Takes the first ', i4, ' values')") mxnlist
!         nlist = mxnlist
!     endif

    ntabtot = ntab + nlist
    
    allocate(rtabular(ntabtot), stat = ierr)
    if (ierr .ne. 0) call error(1,'Memory error when allocating rtabular. Stop')
    
    call sort(nlist,rlist)
    knt = 0
    kntlist = 1
    r = rini
    do i = 1, ntab
        do j = kntlist, nlist
            if (rlist(j) .ge. r) exit
            kntlist = kntlist + 1
            knt = knt+1
            rtabular(knt) = rlist(j)
        enddo
        knt = knt+1
        rtabular(knt) = r
        r = r + dltr
    enddo
    do j = kntlist, nlist
        knt = knt+1
        rtabular(knt) = rlist(j)
    enddo
    ntabtot = min(ntabtot,knt)
    
    call leedamqtpot        !    Reads file .damqt  (generated by DAM2016)

    if (lgbsgz) then
        inquire(file=trim(projectname)//".ggbs", exist=lgbsgz, iostat=ierr)
        if (ierr .eq. 0 .and. lgbsgz) then
            call system ("gzip "//trim(projectname)//".ggbs")
        else
            call system ("gzip "//trim(projectname)//".sgbs")
        endif
    endif
    
    k = 0
    doi: do i = 1, mxsel
        if (iatomsel(i) .le. ncen .and. iatomsel(i) .gt. 0) then
            do j = 1, k
                if (iatomsel(i) .eq. iatomsel(j)) cycle doi
            enddo
            k = k+1
            iatomsel(k) = iatomsel(i)
        endif
    enddo doi
    nsel = k
    if (nsel .eq. 0) then
        nsel = 1
        iatomsel(1) = 1
    endif
    
    call tabula(ltab, mtab, ntabtot, rtabular)
    
    tiempo = dtime(tarray)
    if (longoutput) write(6,"(1x,'Timing in seconds (user, system, total):',/5x,'(',e12.5,',',e12.5,',',e12.5,')')") &
        tarray(1), tarray(2), tiempo
    stop
    end
!**********************************************************************
!    subroutine consta
!
!    Computes and stores auxiliary constants
!        re(i) = dfloat(i)
!        ri(i) = uno / dfloat(i)
!        fact(i) = dfloat(i!)
!        facti(i) = uno / dfloat(i!)
!        facts(i) = dfloat((i+1/2)!)
!
!**********************************************************************
  subroutine consta
    USE DAM_400_D
    USE DAM_400_CONST_D
    USE DAM_400_DATA_D
    USE DAMPOT_400_D

    implicit none
    integer(KINT) :: i, ierr, k1, k12, l, l1, l1l1, l2, l2l2, lm
    integer(KINT) :: m, m1, m1a, m2, m2a, ma, mb, md, ms
    real(KREAL) :: aux, ss, sd
!    auxiliary parameters and functions
    pi = acos(-uno)
    raizpi = sqrt(pi)
    re(0) = cero
    ri(0) = 1.d300
    dosl1(0) = uno
    dosl1i(0) = uno
    do i = 1, mxreal
        re(i) = re(i-1) + uno        ! dfloat(i)
        re(-i) = -re(i)
        ri(i) = uno / re(i)           ! uno / dfloat(i)
        ri(-i) = -ri(i)
        dosl1(i) = re(i) + re(i) + uno    ! dfloat(iì)
        dosl1(-i) = -re(i) - re(i) + uno
        dosl1i(i) = uno / dosl1(i)        ! dfloat( 1/(i+i+1) )
        dosl1i(-i) = uno / dosl1(-i)
    enddo
    fact(0) = uno
    facti(0) = uno
    facts(-1) = raizpi
    facts(0) = facts(-1) * umed
    do i = 1, mxfact
        fact(i) = fact(i-1) * re(i)           !  i!
        facts(i) = facts(i-1) * re(i+i+1) * umed    ! (i+1/2)!
        facti(i) = uno / fact(i)             !  uno / i!
    enddo
    root(0) = cero
    do i = 1, mxroot
        root(i) = sqrt(re(i))        !  sqrt(i)
        rooti(i) = uno / root(i)     !  uno / sqrt(i)
    enddo
    return
    end
!
!    ***************************************************************
!
  subroutine leedamqtpot
    USE DAM_400_D
    USE DAM_400_CONST_D
    USE DAM_400_DATA_D
    USE DAMPOT_400_D
    USE GAUSS
    implicit none
    integer(KINT) :: i, ia, icarga, icflm, ierr, indnf, indng, interv, j, jshft, k, k1, k2, knt, kntlm
    integer(KINT) :: l, lenindintrv, lm, m, nbasis, ncenbas, ncflm, nsamples, nsize
    real(KREAL) :: aux, bux, dltsample, dost, flm, fr1, fr2l2, pi4d2l1, r, ra, ral1inv, rainv, ral
    real(KREAL) :: rinta, rintb, rlarex, step, stepmed, suml, suml1, suml2, summ, summ1, summ2, t
    real(KREAL) :: tcheb(0:mxlenpol-1)
    inquire(file=trim(projectname)//"_2016.damqt", size=nsize, iostat=ierr)
    if (ierr .ne. 0) call error(ierr,'Error when inquiring file '//trim(projectname)//"_2016.damqt")
    if (nsize .eq. -1) call error(1,'Size of file '//trim(projectname)//"_2016.damqt cannot be determined")
    if (longoutput) write(6,"('Size of file ', a, ' = ', i12)") trim(projectname)//"_2016.damqt", nsize
#if _WIN32
    open (unit=10, file=trim(projectname)//"_2016.damqt", form='binary', action = 'read', carriagecontrol='NONE', iostat=ierr)
    if (ierr .ne. 0) call error(1,'Memory error when opening file '//trim(projectname)//"_2016.damqt")
    open (unit=11, file=trim(projectname)//"_2016.dmqtv", form='binary', action = 'read', carriagecontrol='NONE', iostat=ierr)
    if (ierr .ne. 0) call error(1,'Memory error when opening file '//trim(projectname)//"_2016.dmqtv")
#elif __INTEL_COMPILER
    open (unit=10, file=trim(projectname)//"_2016.damqt", form='binary', action = 'read', carriagecontrol='NONE', iostat=ierr)
    if (ierr .ne. 0) call error(1,'Memory error when opening file '//trim(projectname)//"_2016.damqt")
    open (unit=11, file=trim(projectname)//"_2016.dmqtv", form='binary', action = 'read', carriagecontrol='NONE', iostat=ierr)
    if (ierr .ne. 0) call error(1,'Memory error when opening file '//trim(projectname)//"_2016.dmqtv")
#else
    open (unit=10, file=trim(projectname)//"_2016.damqt", form='unformatted', action = 'read', access='stream', iostat=ierr)
    if (ierr .ne. 0) call error(1,'Memory error when opening file '//trim(projectname)//"_2016.damqt")
    open (unit=11, file=trim(projectname)//"_2016.dmqtv", form='unformatted', action = 'read', access='stream', iostat=ierr)
    if (ierr .ne. 0) call error(1,'Memory error when opening file '//trim(projectname)//"_2016.dmqtv")
#endif
    if (longoutput) write(6,"('Opens files ', a, ' and ', a)") trim(projectname)//"_2016.damqt", trim(projectname)//"_2016.dmqtv"
    read(10) ncen, nbas, ncaps
    nsize = nsize - sizeof(ncen) - sizeof(nbas) - sizeof(ncaps)
    if (longoutput) write(6,"('ncen = ', i4, ' nbas = ', i6, ' nshells = ', i5)") ncen, nbas, ncaps

!    Allocates memory for geometry

    allocate(atmnam(ncen), stat = ierr)
    if (ierr .ne. 0) call error(1,'Memory error when allocating atmnam. Stop')

    allocate(nzn(ncen), stat = ierr)
    if (ierr .ne. 0) call error(1,'Memory error when allocating nzn. Stop')

    allocate(rcen(3,ncen), stat = ierr)
    if (ierr .ne. 0) call error(1,'Memory error when allocating rcen. Stop')

    allocate(zn(ncen), stat = ierr)
    if (ierr .ne. 0) call error(1,'Memory error when allocating zn. Stop')

    if (longoutput) write(6,"(/24x,'GEOMETRY (BOHR)')")
    if (longoutput) write(6,"(/t1, ' no. of center:', t20, 'x', t32, 'y', t44, 'z', t56, 'charge')")
    do ia = 1, ncen
        read(10) rcen(1,ia), rcen(2,ia), rcen(3,ia), zn(ia)
        if (abs(zn(ia)-re(int(zn(ia) + umbrzn))) .gt. umbrzn) then
                nzn(ia) = 0
        else
                nzn(ia) = int(zn(ia) + umbrzn)
        endif
        atmnam(ia) = atmnms(nzn(ia))
        if (longoutput) write(6,"(t4, i5, t13, f12.7, t25, f12.7, t37, f12.7, t51, f10.5)") &
                ia, rcen(1,ia), rcen(2,ia), rcen(3,ia) , zn(ia)
    enddo
    nsize = nsize - sizeof(rcen) - sizeof(zn)

    
!    Basis set
    read(10) lsto    ! .true. means STO basis, .false. means GTO basis
    if (lsto) then
        if (lexact) call error(1,'Exact potential not prepared for STO yet. Stop')
!        Allocates memory for the basis set

        allocate(ll(ncaps), stat = ierr)
        if (ierr .ne. 0) call error(1,'Memory error when allocating ll. Stop')

        allocate(lmaxc(ncen), stat = ierr)
        if (ierr .ne. 0) call error(1,'Memory error when allocating lmaxc. Stop')

        allocate(nf(ncaps), stat = ierr)
        if (ierr .ne. 0) call error(1,'Memory error when allocating nf. Stop')

        allocate(ngini(ncen), stat = ierr)
        if (ierr .ne. 0) call error(1,'Memory error when allocating ngini. Stop')

        allocate(ngfin(ncen), stat = ierr)
        if (ierr .ne. 0) call error(1,'Memory error when allocating ngfin. Stop')

        allocate(nn(ncaps), stat = ierr)
        if (ierr .ne. 0) call error(1,'Memory error when allocating nn. Stop')

        allocate(rnor(ncaps), stat = ierr)
        if (ierr .ne. 0) call error(1,'Memory error when allocating rnor. Stop')

        allocate(xx(ncaps), stat = ierr)
        if (ierr .ne. 0) call error(1,'Memory error when allocating xx. Stop')

        if (longoutput) write(6,"(/t22,'STO Basis set',/t22,13('-'))")
        i = 0
        ncenbas = 0
        do ia = 1, ncen
            read(10) ngini(ia), ngfin(ia)
            nsize = nsize - sizeof(ngini(ia)) - sizeof(ngfin(ia))
            if (longoutput) write(6,"(t5,'center ', i4,/t12,'n', t16, 'l', t25,'exp', t35, 'ind func')") ia
            lmaxc(ia) = 0
            if (ngini(ia) .le. 0) cycle
            ncenbas = ncenbas + 1
            do k = ngini(ia), ngfin(ia)
                i = i + 1
                read(10) nf(i), nn(i), ll(i), xx(i)
                nsize = nsize - sizeof(nf(i)) - sizeof(nn(i)) - sizeof(ll(i)) - sizeof(xx(i))
                rnor(i) = sqrt((dos * xx(i))**(2*nn(i)+1) / fact(2*nn(i)))
                if (ll(i) .gt. lmaxc(ia)) lmaxc(ia) = ll(i)
                if (longoutput) write(6,"(t11,i2,t15,i2,t20,e12.5,t36,i4)") nn(i), ll(i), xx(i), nf(i)
            enddo
        enddo
    else
        read(10) nprimitot
        nsize = nsize - sizeof(nprimitot)

!        Allocates memory for the basis set

        allocate(cfcontr(nprimitot), stat = ierr)
        if (ierr .ne. 0) call error(1,'Memory error when allocating cfcontr. Stop')

        allocate(ipntprim(ncaps), stat = ierr)
        if (ierr .ne. 0) call error(1,'Memory error when allocating ipntprim. Stop')

        allocate(ll(ncaps), stat = ierr)
        if (ierr .ne. 0) call error(1,'Memory error when allocating ll. Stop')

        allocate(lmaxc(ncen), stat = ierr)
        if (ierr .ne. 0) call error(1,'Memory error when allocating lmaxc. Stop')

        allocate(ncontr(ncen), stat = ierr)
        if (ierr .ne. 0) call error(1,'Memory error when allocating ncontr. Stop')

        allocate(nf(ncaps), stat = ierr)
        if (ierr .ne. 0) call error(1,'Memory error when allocating nf. Stop')

        allocate(ngini(ncen), stat = ierr)
        if (ierr .ne. 0) call error(1,'Memory error when allocating ngini. Stop')

        allocate(ngfin(ncen), stat = ierr)
        if (ierr .ne. 0) call error(1,'Memory error when allocating ngfin. Stop')

        allocate(nprimit(ncaps), stat = ierr)
        if (ierr .ne. 0) call error(1,'Memory error when allocating nprimit. Stop')

        allocate(rnor(ncaps), stat = ierr)
        if (ierr .ne. 0) call error(1,'Memory error when allocating rnor. Stop')

        allocate(xxg(nprimitot), stat = ierr)
        if (ierr .ne. 0) call error(1,'Memory error when allocating xxg. Stop')

        lmaxbase = 0
        knt = 0
        icarga = 0
        indnf = 1
        indng = 1
        ncenbas = 0
        do ia = 1, ncen
            lmaxc(ia) = 0
            read(10) ncontr(ia)
            nsize = nsize - sizeof(ncontr(ia))
            if (ncontr(ia) .le. 0) then
                ngini(ia) = -1
                ngfin(ia) = -1
                cycle
            endif
            ncenbas = ncenbas + 1
            ngini(ia) = indng
            ngfin(ia) = indng + ncontr(ia) - 1
            indng = indng + ncontr(ia)
            do j = 1, ncontr(ia)
                knt = knt + 1
                read(10) nprimit(knt), ll(knt)
                nsize = nsize - sizeof(nprimit(knt)) - sizeof(ll(knt))
                if (ll(knt) .gt. lmaxc(ia)) lmaxc(ia) = ll(knt)
                nf(knt) = indnf
                indnf = indnf + 2*ll(knt) + 1
                ipntprim(knt) = icarga+1
                read(10) xxg(icarga+1:icarga+nprimit(knt))
                nsize = nsize - sizeof(xxg(icarga+1:icarga+nprimit(knt)))
                read(10) cfcontr(icarga+1:icarga+nprimit(knt))
                nsize = nsize - sizeof(cfcontr(icarga+1:icarga+nprimit(knt)))
!                computes and stores the radial normalization factor
                aux = cero
                bux = ll(knt) + 1.5d0
                do k1 = 1, nprimit(knt)
                    do k2 = 1, k1-1
                        aux=aux + dos*cfcontr(icarga+k1)*cfcontr(icarga+k2)/(xxg(icarga+k1)+xxg(icarga+k2))**bux
                    enddo
                    aux = aux + cfcontr(icarga+k1) * cfcontr(icarga+k1) / (dos*xxg(icarga+k1))**bux
                enddo
                rnor(knt) = sqrt( dos / (facts(ll(knt))*aux) )
                icarga = icarga+nprimit(knt)    ! actualizes the index for loading primitives exponents and contraction coefficients
            enddo
        enddo
        if (longoutput) write(6,"(/t22,'GTO Basis set',/t22,13('-'))")
        if (longoutput) then
            icarga = 0
            knt = 0
            do ia = 1, ncen
                if (ncontr(ia) .le. 0) cycle
                write(6,"(/1x,'atom no.',1x,i4,'(',a2,')')") ia, atmnam(ia)
                write(6,"(1x,'number of contractions = ',i4)") ncontr(ia)
                do j = 1, ncontr(ia)
                    knt = knt + 1
                    write(6,"(/1x,'contraction no. ',i4,' ; l = ',i2)") j,  ll(knt)
                    write(6,"('exponents: ', 8(1x,e12.5))") (xxg(icarga+k), k = 1, nprimit(knt))
                    write(6,"('coefficients: ', 8(1x,e12.5))") (cfcontr(icarga+k),k=1,nprimit(knt))
                    icarga = icarga+nprimit(knt)
                enddo
            enddo
            write(6,"('Number of basis functions = ', i4)") nbas
        endif
    endif

!    Data of density representation
    read(10) lmaxexp
    nsize = nsize - sizeof(lmaxexp)
    if (lmaxrep .gt. lmaxexp) then
        if (longoutput) write(6,"('lmaxrep = ', i3, ' greater than lmaxexp ', i3)") lmaxrep, lmaxexp
        if (longoutput) write(6,"('takes lmaxrep = ',i3)") lmaxexp
        lmaxrep = lmaxexp
    endif
    lmtop = (lmaxexp+1)*(lmaxexp+1)

    if (longoutput) write(6,"('lmaxexp = ', i2, ' nintervaj = ', i2)") lmaxexp, nintervaj

    allocate(icfposd(lmtop*nintervaj+1,ncen), stat = ierr)
    if (ierr .ne. 0) call error(1,'Memory error when allocating icfposd. Stop')
    if (longoutput) write(6,"('Size of icfposd   = ', i15, ' bytes')") size(icfposd)
    nsize = nsize - sizeof(icfposd(:,1)) * ncenbas

    allocate(xajustd(nintervaj,ncen), stat = ierr)
    if (ierr .ne. 0) call error(1,'Memory error when allocating xajustd. Stop')
    if (longoutput) write(6,"('Estimated highest size of xajustd   = ', i15, ' bytes')") size(xajustd)
    nsize = nsize - sizeof(xajustd(:,1)) * ncenbas

    allocate(cfajust(nsize/8), stat = ierr)
    if (ierr .ne. 0) call error(1,'Memory error when allocating cfajust. Stop')
    if (longoutput) write(6,"('Size of cfajust   = ', i15, ' bytes')") size(cfajust)

    if (longoutput) write(6,"('radii of fitting intervals: ',/, 8(1x,e17.10))") rinterv
    icfposd = 0
    xajustd = cero
    cfajust = cero
    k = 0
    do ia = 1, ncen      ! Do over centers
        if (ngini(ia) .le. 0) cycle 
        read(10) icfposd(1:lmtop*nintervaj+1,ia)
        if (k .gt. 0) icfposd(1:lmtop*nintervaj+1,ia) = icfposd(1:lmtop*nintervaj+1,ia) + icfposd(lmtop*nintervaj+1,k) - 1
        k = ia
        read(10) xajustd(1:nintervaj,ia)        ! Exponents
        if (longoutput) write(6,"('fitting exponents: ',/, 8(1x,e17.10))")  xajustd(1:nintervaj,ia)
!     fitting coeficients
        read(10) cfajust(icfposd(1,ia):icfposd(lmtop*nintervaj+1,ia)-1)
    enddo

!    Generates an auxiliary index array for determining the interval to which a given r belongs
    step = rinterv(1)
    fct = uno / step
    lenindintrv = int(rinterv(nintervaj) * fct + udec)

    allocate(indintrv(lenindintrv), stat = ierr)
    if (ierr .ne. 0) call error(1,'Memory error when allocating indintrv. Stop')
    if (longoutput) write(6,"('Size of indintrv   = ', i15, ' bytes')") size(indintrv)

    r = cero
    interv = 1
    do i = 1, lenindintrv-1
        r = r + step
        if (r .gt. (rinterv(interv))) interv = interv + 1
        indintrv(i) = interv
    enddo
    indintrv(lenindintrv) = interv

!    Reads auxiliary integrals from file .dmqtv

    allocate(cfrint1(icfposd(lmtop*nintervaj+1,ncen)-1), stat = ierr)
    if (ierr .ne. 0) call error(1,'Memory error when allocating cfrint1. Stop')
    if (longoutput) write(6,"('Size of cfrint1   = ', i15, ' bytes')") size(cfrint1)

    allocate(cfrint2l2(icfposd(lmtop*nintervaj+1,ncen)-1), stat = ierr)
    if (ierr .ne. 0) call error(1,'Memory error when allocating cfrint2l2. Stop')
    if (longoutput) write(6,"('Size of cfrint2l2 = ', i15, ' bytes')") size(cfrint2l2)

    allocate(QGacum(nintervaj*lmtop,ncen), stat = ierr)
    if (ierr .ne. 0) call error(1,'Memory error when allocating QGacum. Stop')
    if (longoutput) write(6,"('Size of QGacum    = ', i15, ' bytes')") size(QGacum)

    allocate(Qgpart(nintervaj*lmtop), stat = ierr)
    if (ierr .ne. 0) call error(1,'Memory error when allocating Qgpart. Stop')
    if (longoutput) write(6,"('Size of Qgpart    = ', i15, ' bytes')") size(Qgpart)

    allocate(qppart(nintervaj*lmtop), stat = ierr)
    if (ierr .ne. 0) call error(1,'Memory error when allocating qppart. Stop')
    if (longoutput) write(6,"('Size of qppart    = ', i15, ' bytes')") size(qppart)

    allocate(qpacum(nintervaj*lmtop,ncen), stat = ierr)
    if (ierr .ne. 0) call error(1,'Memory error when allocating qpacum. Stop')
    if (longoutput) write(6,"('Size of qpacum    = ', i15, ' bytes')") size(qpacum)

    allocate(rlargo(ncen), stat = ierr)
    if (ierr .ne. 0) call error(1,'Memory error when allocating rlargo. Stop')

    allocate(rmultip(lmtop,ncen), stat = ierr)
    if (ierr .ne. 0) call error(1,'Memory error when allocating rmultip. Stop')
    if (longoutput) write(6,"('Size of rmultip    = ', i15, ' bytes')") size(rmultip)

    cfrint1 = cero
    cfrint2l2 = cero
    QGacum = cero
    Qgpart = cero
    qppart = cero
    qpacum = cero
    rlargo = cero
    rmultip = cero
    do ia = 1, ncen      ! Do over centers
        if (ngini(ia) .le. 0) cycle
!    multipolar moments
        read(11) rmultip(1:lmtop,ia)
!    Reads the integrals:
!        Qg(la,ma,i;ia) = 
!            Integrate[ r**(2*la+2) * fradtr[la,ma,r], {r,l_(i-1),l_i}]
!        qp(la,ma,i;ia) = 
!            Integrate[ r * fradtr[la,ma,r], {r,l_(i-1),l_i}]
!    and computes from them and stores the integrals:
!        QGacum(la,ma,i;ia) = 
!            Integrate[ r**(2*la+2) * fradtr[la,ma,r], {r,0,l_i}]
!        qpacum(la,ma,i;ia) = 
!            Integrate[ r * fradtr[la,ma,r], {r,l_(i-1),Infinity}]
!    where  ia  labels the center (atom),
        read(11) Qgpart(1:nintervaj*lmtop)
        read(11) qppart(1:nintervaj*lmtop)
        QGacum(1:lmtop,ia) = Qgpart(1:lmtop)
        do i = lmtop+1, nintervaj*lmtop, lmtop
            QGacum(i:i+lmtop-1,ia) = QGacum(i-lmtop:i-1,ia) + Qgpart(i:i+lmtop-1)
        enddo
        qpacum((nintervaj-1)*lmtop+1:nintervaj*lmtop,ia) = cero
        do i = (nintervaj-2)*lmtop+1, 1, -lmtop
            qpacum(i:i+lmtop-1,ia) = qpacum(i+lmtop:i+2*lmtop-1,ia) + qppart(i+lmtop:i+2*lmtop-1)
        enddo
!    Reads the fitting coeficients of auxiliary integrals for electrostatic potential and field
        read(11) cfrint1(icfposd(1,ia):icfposd(lmtop*nintervaj+1,ia)-1)    ! Expansion coefficients of auxiliary integrals rint1
        read(11) cfrint2l2(icfposd(1,ia):icfposd(lmtop*nintervaj+1,ia)-1)    ! Expansion coefficients of auxiliary integrals rint2l2
    enddo
    close(10)
    close(11)

!    Determines the long-range radii and the highest l in the expansion for each interval
    allocate(lcorto(nintervaj,ncen), llargo(0:mxlargo,ncen), Qllargo(0:lmaxrep), stat = ierr )
    if (ierr .ne. 0) call error(1,'Memory error when allocating lcorto, llargo and Qllargo. Stop')
    if (longoutput) write(6,"('Size of lcorto   = ', i15, ' bytes')") size(lcorto)
    if (longoutput) write(6,"('Size of llargo   = ', i15, ' bytes')") size(llargo)
    if (longoutput) write(6,"('Size of Qllargo   = ', i15, ' bytes')") size(Qllargo)
!    long-range radii

    allocate(umedpow(0:lmaxexp), stat = ierr)
    if (ierr .ne. 0) call error(ierr,'Memory error when allocating umedpow in leedamqtpot. Stop')
    umedpow(0) = uno                            !
    do i = 1, lmaxexp                           !
            umedpow(i) = umedpow(i-1) * umed    ! 1 / 2^i
    enddo
    if (.not. lexact) then
        if (longoutput) write(6,"('Long-range threshold = ', e12.5)") umbrlargo
        if (longoutput) write(6,"('Highest l in expansion of the potential = ', i2)") lmaxrep
    endif
    umbrlargo2 = umbrlargo*umbrlargo
    nsamples = 4
    lcorto = 0
    llargo = 0
    do ia = 1, ncen
        rlargo(ia) = cero
        if (ngini(ia) .le. 0) cycle
        kntlm = 0
        do l = 0, lmaxrep
            summ1 = cero
            do m = -l, l
                kntlm = kntlm + 1
                summ1 = summ1 + abs(rmultip(kntlm,ia)) * fact(l+abs(m)) * umedpow(abs(m)) &
                                        * facti(l-abs(m)) * facti(abs(m))
            enddo
            Qllargo(l) = summ1
        enddo
        rlargo(ia) = rinterv(nintervaj)
        lcorto(1:nintervaj,ia) = 0
        do interv = 1, nintervaj
            dltsample = udec * (rinterv(interv) - rinterv(interv-1))
            do i = 0, nsamples-1    ! samples over nsamples points in each interval to decide the highest l and long-range radius
                ra = rinterv(interv-1) + dltsample + (rinterv(interv) - rinterv(interv-1) - dos * dltsample) &
                        * ri(nsamples-1) * re(i)
                t = dos * (ra - rinterv(interv-1))/(rinterv(interv)-rinterv(interv-1)) - uno
                dost = t + t
                tcheb(0) = uno    ! Chebyshev T  polynomials
                tcheb(1) = t
                do j = 2, mxlenpol-1
                        tcheb(j) = dost * tcheb(j-1) - tcheb(j-2)
                enddo
                lm = 0
                rainv = uno / ra
                ral = uno
                ral1inv = rainv
                suml1 = cero
                suml2 = cero
                do l = 0, lmaxrep
                    pi4d2l1 = cuatro * pi * dosl1i(l)
                    summ2 = cero
                    do m = -l, l
                        lm = lm + 1
                        if(abs(QGacum((nintervaj-1)*lmtop+lm,ia)) .lt. umbrlargo2) cycle
                        icflm = icfposd((interv-1)*lmtop+lm,ia)
                        rinta = cero
                        rintb = cero
                        do j = 0, icfposd((interv-1)*lmtop+lm+1,ia)-icflm-1
                                rinta = rinta + cfrint2l2(icflm+j) * tcheb(j)
                                rintb = rintb + cfrint1(icflm+j) * tcheb(j)
                        enddo
                        rinta = rinta * (ra-rinterv(interv-1))
                        if (interv .gt. 1) rinta = QGacum((interv-2)*lmtop+lm,ia) + rinta
                        rintb = qpacum((interv-1)*lmtop+lm,ia) + rintb * (rinterv(interv)-ra)
                        summ2 = summ2 + abs(pi4d2l1 * ( ral1inv * rinta + ral * rintb)) * fact(l+abs(m)) &
                                * umedpow(abs(m)) * facti(l-abs(m)) * facti(abs(m))
                    enddo
                    suml1 = suml1 + Qllargo(l) * ral1inv
                    suml2 = suml2 + summ2
                    if (abs(summ2) .gt. umbrlargo2 .and. l .gt. lcorto(interv,ia)) lcorto(interv,ia) = l
                    if (abs(suml2-suml1) .gt. umbrlargo2) rlargo(ia) = rinterv(interv)
                    ral = ral * ra
                    ral1inv = ral1inv * rainv
                enddo
            enddo
        enddo
        if (longoutput) write(6,"('Long-range radius for center ',i4,' (',a2,') = ', e12.5, ' lcorto = ', 30(i3))") &
                    ia, atmnms(nzn(ia)), rlargo(ia), lcorto(1:nintervaj,ia)
        llargo(0,ia) = lcorto(1,ia)
        do i = 1, mxlargo
            ra = re(i)
            rainv = uno / ra
            ral1inv = rainv
            kntlm = 0
            do l = 0, lmaxrep
                summ1 = cero
                do m = -l, l
                    kntlm = kntlm + 1
                    summ1 = summ1 + abs(rmultip(kntlm,ia)) * fact(l+abs(m)) * umedpow(abs(m)) &
                                            * facti(l-abs(m)) * facti(abs(m))
                enddo
                Qllargo(l) = summ1 * ral1inv
                ral1inv = ral1inv * rainv
            enddo
            llargo(i,ia) = 0
            suml1 = cero
            do l = lmaxrep, 0, -1
                suml1 = suml1 + Qllargo(l)
                if (suml1 .gt. umbrlargo2) then
                    llargo(i,ia) = l
                    exit
                endif
            enddo
        enddo
        if (longoutput) write(6,"('llargo: ', 51(i3))") llargo(0:mxlargo,ia)
    enddo
    deallocate (Qgpart, qppart)
    deallocate(umedpow)
    return
    end
!
!   ***************************************************************
!     Calculates the electrostatic potential from the represented density at point (x,y,z)
!
   subroutine tabula(l, m, ntabtot, rtabular)
    USE DAM_400_DATA_D
    USE DAMPOT_400_D
    USE SELFREPULSION
    implicit none
    integer(KINT) :: i, ia, ierr, interv, icflm, j, jshft, k, kntlm, l, lm, lmtopot, ltop, m, ntabtot
    real(KREAL) :: cf, dost, ra, rant, rintb, step, suma, t
    real(KREAL) :: tcheb(0:mxlenpol-1)
    real(KREAL), allocatable :: flmtab(:)
    real(KREAL) :: rtabular(*)

    allocate(flmtab(ntabtot), stat = ierr)
    if (ierr .ne. 0) call error(1,'Memory error when allocating flmtab. Stop')
    
    if (ngini(iatomsel(1)) .eq. 0) then   ! center without basis set
        write(6,"('This center has no density associated to it. Cannot tabulate.')")
        return
    endif
!    write(6,"(4(3x,i5))") ntabtot
!    write(6,"(3x,7(1x,e17.10))") rtabular(1:ntabtot)

    do k = 1, nsel
        ia = iatomsel(k)
        if (ngini(ia) .le. 0) then
            write(6,"('Center ', i4, ' has no associated radial factors')") ia
            cycle
        endif
        write(6,"(/10x,'Self-repulsion radial factor of center ',i2,' (',a2,'): frep(l,m,r)', &
                 //11x,'r',t24,'frep(',i2,',',i3,',r)', t43 &
                , '(r**',i2,') * frep(',i2,',',i3,',r)', /5x,70('-'))") ia, atmnam(ia), l, m, l, l, m
    
        lm = l**2 + l + m + 1

        if(icfposd(lm+1,ia) .le. icfposd(lm,ia)) then    ! If the radial factor is negligible, prints a message and exits
            write(6,"('Negligible radial factor')")
            cycle
        endif


        step = rinterv(1)
        fct = uno / step

        cf = dos * pi * sqrt(dosl1i(l)*dos * fact(l+abs(m))/fact(l-abs(m)))
        if (m .eq. 0) cf = cf * root(2)

        suma = cero
        rant = cero
        do i = 1, ntabtot
            ra = rtabular(i)
            if (ra .ge. rlargo(ia)) then  ! The point is in the long-range region (ra >= rlargo(ia) )
                flmtab(i) = 0.d0
            else        !     The point is in the short-range region (ra < rlargo(ia) )
                interv = indintrv(int(fct*ra)+1)
                t = dos * (ra - rinterv(interv-1))/(rinterv(interv)-rinterv(interv-1)) - uno
                dost = t + t
                tcheb(0) = uno    ! Chebyshev T  polynomials
                tcheb(1) = t
                do j = 2, mxlenpol-1
                    tcheb(j) = dost * tcheb(j-1) - tcheb(j-2)
                enddo

                if(abs(QGacum((nintervaj-1)*lmtop+lm,ia)) .lt. umbrlargo2) then
                    flmtab(i) = 0.d0
                    cycle
                endif
                icflm = icfposd((interv-1)*lmtop+lm,ia)
                rintb = cero
                do j = 0, icfposd((interv-1)*lmtop+lm+1,ia)-icflm-1
                    rintb = rintb + cfrint1(icflm+j) * tcheb(j)
                enddo
                flmtab(i) = (qpacum((interv-1)*lmtop+lm,ia) + rintb * (rinterv(interv)-ra))
                suma = suma + (cf* ra**l * flmtab(i))**2 * (ra-rant)
                rant = ra
            endif
            write(6,"(3x,f12.5,3x,e22.15,3x,e22.15)") ra, flmtab(i),  (cf * ra**l * flmtab(i) )**2
        enddo
        write(6,"(/,7x,70('-'),/,7x,'TOTAL SUM',t44,e22.15)") suma
    enddo
    return
    end
!
!	*************************************************************
!
  subroutine sort(n, xsort)
    USE DAM_400_D
    implicit none
    integer(KINT) :: i, j, n
    real(KREAL) :: aux
    integer(KINT), parameter :: mxsteps = 1000
    logical :: lend
    real(KREAL) :: xsort(*)
    do i = 1, mxsteps
        lend = .true.
        do j = 1, 2*(n/2), 2
            if (xsort(j) .le. xsort(j+1)) cycle
            aux = xsort(j)
            xsort(j) = xsort(j+1)
            xsort(j+1) = aux
            lend = .false.
        enddo
        do j = 2, 2*((n-1)/2), 2
            if (xsort(j) .le. xsort(j+1)) cycle
            aux = xsort(j)
            xsort(j) = xsort(j+1)
            xsort(j+1) = aux
            lend = .false.
        enddo
        if (lend) return
    enddo
    call error(1,'Highest number of steps in sort reached. Stop')
    return
    end
!
!    -------------------------------------------------------------------------------------------------------
!
  subroutine error(ierr, msg)
    USE DAM_400_D
    implicit none
    integer(KINT) :: ierr
    character(*) :: msg
    write(6,"(a)") msg
    write(6,"('Error code = ', i4)") ierr
    stop
    end
