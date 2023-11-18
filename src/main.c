/*
 * Luna - the moon phases for Numworks calculators
 * 
 * Copyright (C) 2013 Federico Pelupessy
 *
 *               Moon picture from Wikipedia, released to public domain,
 *               (by Tomruen, original upload 5 November 2007)
 *               Moon phase code based on moontool.c by John Walker
 *                 extended by: Marek Niemiec, DB1BMN, Dec. 29th. 2009  till Jan. 12 th. 2010
 *               picojpeg by: Rich Geldreich <richgel99@gmail.com>
 * 
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
 * KIND, either express or implied.
 *
 */

#include <eadk.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "luna_data.h"
#include "picojpeg.h"
#include <time.h>
#include "moontool.h"


const char eadk_app_name[] __attribute__((section(".rodata.eadk_app_name"))) = "Luna";
const uint32_t eadk_api_level  __attribute__((section(".rodata.eadk_api_level"))) = 0;

#define NFIELD 5 // 0=hr, 1=min,2=day,3=month,4=yr
const int utc_offset=0;

void statuslinemsg(const char * msg) {
  uint16_t c=0xfda6;
  eadk_display_push_rect_uniform((eadk_rect_t){0,0,320,18}, c);
  eadk_display_draw_string(msg, (eadk_point_t){160-5*strlen(msg), 0}, true, eadk_color_white, c);
}


int get_frame_no(double phase,int nframe, float *frame_phase)
{
    int i,no;
    float d,dd;

    if (phase < 0) phase = 0.;
    if (phase > 1) phase = 1.;

    dd = 2.;
    no = -1;
    for (i = 0;i < nframe; i++)
    {
        d = phase-frame_phase[i];
        if (d >= 0.5) d = d-1;
        if (d < -0.5) d = d+1;
        if (d < 0) d=-d;
        if (d < dd) 
        {
            no = i;
            dd = d;
        } 
    }
    return no;
}

void show_data(struct tm* time)
{
    char buf[100];    
    static const char *desc[] = {
        "New moon", "Waxing Crescent",
        "First Quarter","Waxing Gibbous",
        "Full Moon","Waning Gibbous","Last Quarter",
        "Waning Crescent"
    };
    int iphase;
    double phase,jd,cphase, aom, cdist, cangdia, csund, csuang;
    double jdfull,jdnew;
    char phase_tendency;
    struct tm utc, tmfull,tmnew;

    utc=tm2utc(time,utc_offset);
    jd=jtime(&utc);
    phase=moon_phase(jd, &cphase, &aom, &cdist, &cangdia, &csund, &csuang);

    eadk_display_push_rect_uniform((eadk_rect_t){0,18,EADK_SCREEN_WIDTH,EADK_SCREEN_HEIGHT-36}, 0x0);

    iphase=8*phase+0.5;
    iphase=iphase%8;

    int x=10;  
    int y=20;
    
    sprintf(buf, "Julian date:");
    eadk_display_draw_string(buf, (eadk_point_t){x, y}, true, 0xfda6, eadk_color_black);
    y+=20;

    sprintf(buf, "   %ld.%04ld",(long) jd, (long) (10000*(jd- (long) jd)));
    eadk_display_draw_string(buf, (eadk_point_t){x, y}, true, 0xfda6, eadk_color_black);
    y+=20;


    if (phase <= 0.5)
    {
       phase_tendency = '+'; 
    }
    else
    {
       phase_tendency = '-'; 
    }

    sprintf(buf, "Moon phase:");
    eadk_display_draw_string(buf, (eadk_point_t){x, y}, true, 0xfda6, eadk_color_black);
    y+=20;

    sprintf(buf, "   %s (%d%% %c)",desc[iphase], (int)(cphase*100+0.5), phase_tendency);
    eadk_display_draw_string(buf, (eadk_point_t){x, y}, true, 0xfda6, eadk_color_black);
    y+=20;

    sprintf(buf, "Distance:");
    eadk_display_draw_string(buf, (eadk_point_t){x, y}, true, 0xfda6, eadk_color_black);
    y+=20;

    sprintf(buf, "   %ld km",(long)(cdist));
    eadk_display_draw_string(buf, (eadk_point_t){x, y}, true, 0xfda6, eadk_color_black);
    y+=20;



    jdfull=phase_search_forward(jd,0.5);  
    jdnew=phase_search_forward(jd,1.0);  
    tmfull=timej(jdfull+utc_offset/24);
    tmnew=timej(jdnew+utc_offset/24);

    int ynew,yfull;

    if(jdnew<jdfull) 
    {
      ynew=y;
      yfull=ynew+40;
    } else
    {
      yfull=y;
      ynew=yfull+40;

    }
    sprintf(buf, "Next new moon:");
    eadk_display_draw_string(buf, (eadk_point_t){x, ynew}, true, 0xfda6, eadk_color_black);
    y+=20;      

    sprintf(buf, "   %02d/%02d/%04d, %02d:%02d:%02d (UTC)",
     tmnew.tm_mday,tmnew.tm_mon+1,tmnew.tm_year+1900,tmnew.tm_hour, tmnew.tm_min, tmnew.tm_sec);
    eadk_display_draw_string(buf, (eadk_point_t){x, ynew+20}, true, 0xfda6, eadk_color_black);
    y+=20;      

    sprintf(buf, "Next full moon:");
    eadk_display_draw_string(buf, (eadk_point_t){x, yfull}, true, 0xfda6, eadk_color_black);
    y+=20;

    sprintf(buf, "   %02d/%02d/%04d, %02d:%02d:%02d (UTC)",
     tmfull.tm_mday,tmfull.tm_mon+1,tmfull.tm_year+1900, tmfull.tm_hour, tmfull.tm_min, tmfull.tm_sec);
    eadk_display_draw_string(buf, (eadk_point_t){x, yfull+20}, true, 0xfda6, eadk_color_black);
    y+=20;
            
}

int jpg_offset, jpg_end;

unsigned char pjpeg_need_bytes_callback(unsigned char* pBuf, unsigned char buf_size, unsigned char *pBytes_actually_read, void *pCallback_data)
{
   unsigned int n;
   
   n=buf_size;
   if(n>jpg_end - jpg_offset)n=jpg_end - jpg_offset;   
   memcpy( (void*) pBuf, (void*) Luna_dat+jpg_offset, n);
   *pBytes_actually_read = (unsigned char)(n);
   jpg_offset += n;
   return 0;
}

void show_pic(struct tm* time)
{
  pjpeg_image_info_t image_info;
  eadk_color_t *pixels;

  pixels=(eadk_color_t *) malloc(64);

  unsigned char status;
  double phase,jd,cphase, aom, cdist, cangdia, csund, csuang;
  struct tm utc;

  eadk_display_push_rect_uniform((eadk_rect_t){0,0,EADK_SCREEN_WIDTH,EADK_SCREEN_HEIGHT}, 0x0);

  utc=tm2utc(time,utc_offset);
  jd=jtime(&utc);
  phase=moon_phase(jd, &cphase, &aom, &cdist, &cangdia, &csund, &csuang);
  
  int iphase=get_frame_no(phase,nframe,frame_phases);
  jpg_offset=offsets[iphase];
  jpg_end=offsets[iphase+1];
  
  status = pjpeg_decode_init(&image_info, pjpeg_need_bytes_callback, NULL, 0);
  
  if (status)
  {
    printf("pjpeg_decode_init() failed with status %u\n", status);
    if (status == PJPG_UNSUPPORTED_MODE)
    {
       printf("Progressive JPEG files are not supported.\n");
    }
    return;
  }

  int mcu_y=0;
  int mcu_x=0;
  int x,y;
  unsigned int c;
  for ( ; ; )
  {  
    status = pjpeg_decode_mcu();
    
    if (status)
    {
      if (status != PJPG_NO_MORE_BLOCKS)
        printf("pjpeg_decode_mcu() failed with status %u\n", status);  
      break;
    }
    
    if (mcu_y >= image_info.m_MCUSPerCol)
    {
      printf("image decode finds too many blocks\n");
      return;
    }

    x=8*mcu_x+40;
    y=8*mcu_y;
    for(int i=0;i<64;i++)
    {
      c=image_info.m_pMCUBufR[i];
      pixels[i]=(((uint16_t)(eadk_color_red * c / 255.)) & eadk_color_red)+
                (((uint16_t)(eadk_color_green * c / 255.)) & eadk_color_green)+
                (((uint16_t)(eadk_color_blue * c / 255.)) & eadk_color_blue);
    }
    eadk_display_push_rect((eadk_rect_t){x,y,8,8},(const eadk_color_t *)pixels);
    mcu_x++;
    if (mcu_x == image_info.m_MCUSPerRow)
    {
      mcu_x = 0;
      mcu_y++;
    }
  }
  free((void*)pixels);

}

void show_date(struct tm* time, int field)
{
  char buf[10];
  int c[NFIELD]={eadk_color_black};
  c[field]=0xfda6;

  statuslinemsg("Luna");
  eadk_display_push_rect_uniform((eadk_rect_t){0,EADK_SCREEN_HEIGHT-18,EADK_SCREEN_WIDTH,18}, 0x0);


  int x=50;  
  sprintf(buf, "%02d",time->tm_hour);
  eadk_display_draw_string(buf, (eadk_point_t){x, 222}, true, eadk_color_white, c[0]);
  x+=10*strlen(buf);
  
  strcpy(buf,":");
  eadk_display_draw_string(buf, (eadk_point_t){x, 222}, true, eadk_color_white, eadk_color_black);
  x+=10*strlen(buf);

  sprintf(buf, "%02d",time->tm_min);
  eadk_display_draw_string(buf, (eadk_point_t){x, 222}, true, eadk_color_white, c[1]);
  x+=10*strlen(buf);

  strcpy(buf," (UTC) ");
  eadk_display_draw_string(buf, (eadk_point_t){x, 222}, true, eadk_color_white, eadk_color_black);
  x+=10*strlen(buf);

  sprintf(buf, "%02d",time->tm_mday);  
  eadk_display_draw_string(buf, (eadk_point_t){x, 222}, true, eadk_color_white, c[2]);
  x+=10*strlen(buf);

  strcpy(buf,"/");
  eadk_display_draw_string(buf, (eadk_point_t){x, 222}, true, eadk_color_white, eadk_color_black);
  x+=10*strlen(buf);

  sprintf(buf, "%02d",time->tm_mon+1);
  eadk_display_draw_string(buf, (eadk_point_t){x, 222}, true, eadk_color_white, c[3]);
  x+=10*strlen(buf);

  strcpy(buf,"/");
  eadk_display_draw_string(buf, (eadk_point_t){x, 222}, true, eadk_color_white, eadk_color_black);
  x+=10*strlen(buf);

  sprintf(buf, "%04d",time->tm_year+1900);
  eadk_display_draw_string(buf, (eadk_point_t){x, 222}, true, eadk_color_white, c[4]);
  x+=10*strlen(buf);

}

int new_active_field(eadk_event_t ev, int field)
{
  if(ev==eadk_event_left) field-=1;
  if(ev==eadk_event_right) field+=1;
  if(field<0) field=0;
  if(field>NFIELD-1) field=NFIELD-1;
  return field;
}

void update_time(eadk_event_t ev, struct tm* time, int field)
{
  int incr;
  if(ev==eadk_event_up) incr=1;
  if(ev==eadk_event_down ) incr=-1;
  switch(field)
  {
    case 0:
      time->tm_hour+=incr;
      break;
    case 1:
      time->tm_min+=incr;
      break;
    case 2:
      time->tm_mday+=incr;
      break;
    case 3:
      time->tm_mon+=incr;
      break;
    case 4:
      time->tm_year+=incr;
      break;
  }
  mktime(time);
}

void mainloop() {
  struct tm time={.tm_year=2023-1900, .tm_mday=1};
  int mode=0; //mode=0-> show data, 1-> show pic
  int update=0;
  int current_field=0; 

  show_date(&time,current_field);
  show_data(&time);
  while (true) {
    int32_t timeout=1000;
    eadk_event_t ev=eadk_event_get(&timeout);
    if(ev==eadk_event_back || ev==eadk_key_on_off )  return;
    if(ev==eadk_event_left || ev==eadk_event_right )
    {
      current_field=new_active_field(ev, current_field);
      mode=0;
      update=1;
    }
    if(ev==eadk_event_up || ev==eadk_event_down )
    {
      update_time(ev, &time, current_field);
      mode=0;
      update=1;
    }
    if(ev==eadk_event_ok) 
    {
      mode=1-mode;
      update=1;
    }
    if(update)
    {
      show_date(&time,current_field);
      if(mode) 
        {
          show_pic(&time);
        }
      else
        show_data(&time);
      update=0;
    }
  }
}



int main(int argc, char * argv[]) {
  eadk_display_push_rect_uniform((eadk_rect_t){0,0,EADK_SCREEN_WIDTH,EADK_SCREEN_HEIGHT}, 0x0);
  mainloop();
}
