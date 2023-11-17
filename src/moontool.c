#include <math.h>
#include <time.h>
#include <string.h>

/*  Astronomical constants  */
#define EPOCH       2444238.5      /* 1980 January 0.0 */

/*  Constants defining the Sun's apparent orbit  */

#define ELONGE      278.833540     /* Ecliptic longitude of the Sun at epoch 1980.0 */
#define ELONGP      282.596403     /* Ecliptic longitude of the Sun at perigee */
#define ECCENT      0.016718       /* Eccentricity of Earth's orbit */
#define SUNSMAX     1.495985e8     /* Semi-major axis of Earth's orbit, km */
#define SUNANGSIZ   0.533128       /* Sun's angular size, degrees, at
                                      semi-major axis distance */
/*  Elements of the Moon's orbit, epoch 1980.0  */
#define MMLONG      64.975464      /* Moon's mean longitude at the epoch */
#define MMLONGP     349.383063     /* Mean longitude of the perigee at the epoch */
#define MLNODE      151.950429     /* Mean longitude of the node at the epoch */
#define MINC        5.145396       /* Inclination of the Moon's orbit */
#define MECC        0.054900       /* Eccentricity of the Moon's orbit */
#define MANGSIZ     0.5181         /* Moon's angular size at distance a
                                            from Earth */
#define MSMAX       384401.0       /* Semi-major axis of Moon's orbit in km */
#define MPARALLAX   0.9507         /* Parallax at distance a from Earth */
#define SYNMONTH    29.53058868    /* Synodic month (new Moon to new Moon) */
#define LUNATBASE   2423436.0      /* Base date for E. W. Brown's numbered
                                      series of lunations (1923 January 16) */

/*  Properties of the Earth  */

#define EARTHRAD    6378.16        /* Radius of Earth in kilometres */

/*  Handy mathematical functions  */

#define PI 3.14159265358979323846

#define SGN(x) (((x) < 0) ? -1 : ((x) > 0 ? 1 : 0))       /* Extract sign */
#ifndef ABS
#define ABS(x) ((x) < 0 ? (-(x)) : (x))                   /* Absolute val */
#endif
#define FIXANGLE(a)  ((a) < 0 ? 360.*((a)/360.-(long)((a)/360.)+1) : 360.*((a)/360.-(long)((a)/360.)))
                                                          /* angle to 0-360 */
#define FIXRANGLE(a) ((a) < 0 ? 2*PI*((a)/2/PI-(long)((a)/2/PI)+1) : 2*PI*((a)/2/PI-(long)((a)/2/PI)))
                                                          /* angle to 0-2PI */
#define TORAD(d) ((d) * (PI / 180.0))                     /* Deg->Rad     */
#define TODEG(d) ((d) * (180.0 / PI))                     /* Rad->Deg     */

#define dsin(x) (sin(TORAD((x))))                   /* Sin from deg */
#define dcos(x) (cos(TORAD((x))))                   /* Cos from deg */

/*  JTIME  --  Convert (GMT)  date  and  time  to  astronomical
        Julian   time  (i.e. Julian  date  plus  day  fraction,
        expressed as a double).  */
double jtime(struct tm *t)
{
    return ( mktime(t) / 86400.0 ) + 2440587.5;
}

static time_t mktimej(double jtime)
{
    return (time_t) ((jtime - 2440587.5) * 86400.0);
}

struct tm timej(double jtime)
{
    time_t t;
    t=mktimej(jtime);
    return *gmtime(&t);
}

struct tm tm2utc(struct tm *t, double utc_offset)
{
       struct tm tmp;         
       tmp = timej(jtime(t)- utc_offset/24);
       return tmp;
}

/*  PHASE  --  Calculate phase of moon as a fraction:
        The  argument  is  the  time  for  which  the  phase is
        requested, expressed as a Julian date and fraction.  Returns  the  terminator
        phase  angle  as a percentage of a full circle (i.e., 0 to 1), and
        stores into pointer arguments  the  illuminated  fraction  of the
        Moon's  disc, the Moon's age in days and fraction, the distance
        of the Moon from the centre of the Earth, and  the  angular
        diameter subtended  by the Moon as seen by an observer at the centre of
        the Earth.
*/

double moon_phase(
        double  pdate,                      /* Date for which to calculate phase */
        double  *pphase,                    /* Illuminated fraction */
        double  *mage,                      /* Age of moon in days */
        double  *dist,                      /* Distance in kilometres */
        double  *angdia,                    /* Angular diameter in degrees */
        double  *sudist,                    /* Distance to Sun */
        double  *suangdia)                  /* Sun's angular diameter */
{
    double Day, N, M, Ec, Lambdasun, ml, MM, /* MN,*/ Ev, Ae, A3, MmP,
           mEc, A4, lP, V, lPP,
           /* NP, y, x, Lambdamoon, BetaM, */
           MoonAge, MoonPhase,
           MoonDist, MoonDFrac, MoonAng,
           /* MoonPar,*/
           F, SunDist, SunAng,Mrad;

    /* Calculation of the Sun's position */

    Day = pdate - EPOCH;                    /* Date within epoch */
    N = FIXANGLE((360 / 365.2422) * Day);   /* Mean anomaly of the Sun */
    M = FIXANGLE(N + ELONGE - ELONGP);      /* Convert from perigee
                                               co-ordinates to epoch 1980.0 */
    Mrad=TORAD(M);
    Ec = Mrad+2*ECCENT*sin(Mrad)+
        1.25*ECCENT*ECCENT*sin(2*Mrad)+
        ECCENT*ECCENT*ECCENT*sin(3*Mrad); /* gives true anomaly accurate to ~arcsec
                                        (compared to usual formula, that is) */
    /*    if(Mrad > PI) Ec=Ec-2*PI; */
    Ec = TODEG(Ec);

    Lambdasun = FIXANGLE(Ec + ELONGP);      /* Sun's geocentric ecliptic
                                                longitude */
    /* Orbital distance factor */
    F = (1 + ECCENT * dcos(Ec)) / (1 - ECCENT * ECCENT);
    SunDist = SUNSMAX / F;                  /* Distance to Sun in km */
    SunAng = F * SUNANGSIZ;                 /* Sun's angular size in degrees */

    /* Calculation of the Moon's position */

    /* Moon's mean longitude */
    ml = FIXANGLE(13.1763966 * Day + MMLONG);

    /* Moon's mean anomaly */
    MM = FIXANGLE(ml - 0.1114041 * Day - MMLONGP);

    /* Moon's ascending node mean longitude */
    /*  MN = FIXANGLE(MLNODE - 0.0529539 * Day); */

    /* Evection */
    Ev = 1.2739 * dsin(2 * (ml - Lambdasun) - MM);

    /* Annual equation */
    Ae = 0.1858 * dsin(M);

    /* Correction term */
    A3 = 0.37 * dsin(M);

    /* Corrected anomaly */
    MmP = MM + Ev - Ae - A3;

    /* Correction for the equation of the centre */
    mEc = 6.2886 * dsin(MmP);

    /* Another correction term */
    A4 = 0.214 * dsin(2 * MmP);

    /* Corrected longitude */
    lP = ml + Ev + mEc - Ae + A4;

    /* Variation */
    V = 0.6583 * dsin(2 * (lP - Lambdasun));

    /* True longitude */
    lPP = lP + V;

    /* Corrected longitude of the node */
    /*    NP = MN - 0.16 * dsin(M); */

    /* Y inclination coordinate */
    /*    y = dsin(lPP - NP) * dcos(MINC); */

    /* X inclination coordinate */
    /*    x = dcos(lPP - NP); */

    /* Ecliptic longitude */
    /*    Lambdamoon = TODEG(atan2(y, x)); */
    /*    Lambdamoon += NP; */

    /* Ecliptic latitude */
    /*    BetaM = TODEG(asin(fp_sin(TORAD(lPP - NP)) * fp_sin(TORAD(MINC)))); */

    /* Calculation of the phase of the Moon */

    /* Age of the Moon in degrees */
    MoonAge = lPP - Lambdasun;

    /* Phase of the Moon */
    MoonPhase = (1 - dcos(MoonAge)) / 2;

    /* Calculate distance of moon from the centre of the Earth */

    MoonDist = (MSMAX * (1 - MECC * MECC)) /
                (1 + MECC * dcos(MmP + mEc));

    /* Calculate Moon's angular diameter */

    MoonDFrac = MoonDist / MSMAX;
    MoonAng = MANGSIZ / MoonDFrac;

    /* Calculate Moon's parallax */
    /*    MoonPar = MPARALLAX / MoonDFrac; */

    *pphase = MoonPhase;
    *mage = SYNMONTH * (FIXANGLE(MoonAge) / 360.0);
    *dist = MoonDist;
    *angdia = MoonAng;
    *sudist = SunDist;
    *suangdia = SunAng;
    return FIXANGLE(MoonAge) / 360.0;
}

#define FIXNANGLE(a) ((a) < 0 ? ((a)-(long)((a))+1) : ((a)-(long)((a))))

double phase_search_forward(double jd,double target_phase)
{
    double phase,cphase, aom, cdist, cangdia, csund, csuang;
    double low_phase,jdlow,jdhigh;
    double prec=1./(2*30.*24.*3600);    
    int count;
    
    phase=moon_phase(jd, &cphase, &aom, &cdist, &cangdia, &csund, &csuang);
    low_phase=phase;
    jdlow=jd;
    jdhigh=jd;
    count=0;
    while( FIXNANGLE(phase-low_phase) < FIXNANGLE(target_phase-low_phase) ) 
    {
        count++; if(count>5) return -1.;
        jdlow=jdhigh;
        low_phase=phase;
        jdhigh+=10.;
        phase=moon_phase(jdhigh, &cphase, &aom, &cdist, &cangdia, &csund, &csuang);
    }

    while( ABS(phase - target_phase) > prec)
    {
        count++; if(count>50) return -2.;      
        jd=(jdlow+jdhigh)/2;
        phase=moon_phase(jd, &cphase, &aom, &cdist, &cangdia, &csund, &csuang);
        if( FIXNANGLE(phase-low_phase) < FIXNANGLE(target_phase-low_phase) ) 
        {
            jdlow=jd;
            low_phase=phase;
        } else
        {
            jdhigh=jd;
        }
    }
    return jd;
}



//~ static int show_image(int fd,struct tm *tm,bool refresh, int *next_choice)
//~ {
    //~ static struct bitmap input_bmp={
      //~ .width=PICSIZE,
      //~ .height=PICSIZE,           
      //~ .format=FORMAT_NATIVE
    //~ };
    //~ size_t plugin_buf_len;
    //~ unsigned char *jpeg_buf;
    //~ unsigned char * plugin_buf =
        //~ (unsigned char *)rb->plugin_get_buffer(&plugin_buf_len);
    //~ int font_w,font_h,iphase;
    //~ double phase,jd,cphase, aom, cdist, cangdia, csund, csuang;
    //~ int ret;
    //~ int action;
    //~ struct tm utc;

    //~ utc=tm2utc(tm,utc_offset);
    //~ jd=jtime(&utc);
    //~ phase=moon_phase(jd, &cphase, &aom, &cdist, &cangdia, &csund, &csuang);
    
    //~ if(fd>=0)
    //~ {
        //~ rb->lcd_clear_display();
        //~ rb->lcd_getstringsize( (unsigned char *) "Calculating..", &font_w, &font_h);
        //~ rb->lcd_putsxy((LCD_WIDTH/2) - (font_w)/2, LCD_HEIGHT/2-font_h/2,
                        //~ (unsigned char *) "Calculating..");
        //~ rb->lcd_update();

        //~ if(refresh)
        //~ {
            //~ int i,skip;
            //~ iphase=get_frame_no(phase,nframe,frame_phase);
            //~ skip=0; for(i=0; i<iphase;i++) skip+=frame_size[i];

            //~ if (frame_size[iphase] > plugin_buf_len)
              //~ return PLUGIN_ERROR;

            //~ rb->lseek(fd, (off_t) (skip + foffset),SEEK_SET);

            //~ jpeg_buf=plugin_buf;
            //~ rb->read(fd, jpeg_buf, frame_size[iphase]);

            //~ input_bmp.data=(char*) (plugin_buf+frame_size[iphase]);

            //~ ret = decode_jpeg_mem(jpeg_buf,frame_size[iphase], 
                                    //~ &input_bmp, plugin_buf_len-frame_size[iphase],
                                    //~ FORMAT_NATIVE|FORMAT_RESIZE|FORMAT_KEEP_ASPECT,
                                    //~ &format_native);

            //~ if (ret < 0)
            //~ {
                //~ rb->splash(HZ, "Could not decode image");
                //~ return PLUGIN_ERROR;
            //~ }
        //~ }

        //~ rb->lcd_bitmap( (fb_data *)input_bmp.data,
                        //~ MAX(0,(LCD_WIDTH-input_bmp.width)/2),
                        //~ MAX(0,(LCD_HEIGHT-input_bmp.height)/2),
                        //~ input_bmp.width,input_bmp.height);
        //~ rb->lcd_update();

        //~ action=pluginlib_getaction(TIMEOUT_BLOCK, plugin_contexts, 1);
        //~ if(action==QUIT_ACTION) *next_choice=QUIT;
        //~ return PLUGIN_OK;
    //~ } else
        //~ return PLUGIN_ERROR;
//~ }




