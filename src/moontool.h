double moon_phase(
        double  pdate,                      /* Date for which to calculate phase */
        double  *pphase,                    /* Illuminated fraction */
        double  *mage,                      /* Age of moon in days */
        double  *dist,                      /* Distance in kilometres */
        double  *angdia,                    /* Angular diameter in degrees */
        double  *sudist,                    /* Distance to Sun */
        double  *suangdia);                  /* Sun's angular diameter */

double phase_search_forward(double jd,double target_phase);

double jtime(struct tm *t);
struct tm timej(double jtime);
struct tm tm2utc(struct tm *t, double utc_offset);
