#define FREERTOS
extern "C" {
#include <libesphttpd/platform.h>
#include <libesphttpd/httpd.h>
#include <libesphttpd/httpdespfs.h>
#include <libesphttpd/cgiflash.h>
}
#include "Ws2811.h"

CgiUploadFlashDef uploadParams={
	.type=CGIFLASH_TYPE_FW,
	.fw1Pos=0x2000,
	.fw2Pos=((FLASH_SIZE*1024*1024)/2)+0x2000,
	.fwSize=((FLASH_SIZE*1024*1024)/2)-0x2000,
	LIBESPHTTPD_OTA_TAGNAME
};

struct BlockVars {
    unsigned xgamma:16;
    unsigned delay:16;
    unsigned Wr:16, Wg:16, Wb:16;
    unsigned v1:16;
    unsigned s1:16;
    unsigned begin:16;
    unsigned end:16;
    unsigned v:16;
    unsigned s:16;
}__attribute__((__packed__));

int ICACHE_FLASH_ATTR cgiStripControl(HttpdConnData *connData) {
    if (!connData->conn) { //Connection aborted. Clean up.
        return HTTPD_CGI_DONE;
      }
    switch (connData->requestType) {
    default:
        httpdStartResponse(connData, 406);  //http error code 'unacceptable'
        httpdEndHeaders(connData);
        break;
    case HTTPD_METHOD_POST:
        if (connData->post->len < sizeof (BlockVars) || connData->post->len != connData->post->buffLen) {
            httpdStartResponse(connData, 406);  //http error code 'unacceptable'
            httpdEndHeaders(connData);
            return HTTPD_CGI_DONE;
        }
        {
          const BlockVars* input = reinterpret_cast<const BlockVars*>(connData->post->buff);
          Ws2811::xGammaLowBits = input->xgamma;
          strip_drv_delay = input->delay;
          PlazmaZone::c0.r = input->Wr;
          PlazmaZone::c0.g = input->Wg;
          PlazmaZone::c0.b = input->Wb;
          std::static_pointer_cast<PlazmaZone>(strip.zone[0])->v1 = input->v1;
          std::static_pointer_cast<PlazmaZone>(strip.zone[0])->s1 = input->s1;
          std::static_pointer_cast<PlazmaZone>(strip.zone[0])->begin = input->begin;
          std::static_pointer_cast<PlazmaZone>(strip.zone[0])->end = input->end;
          std::static_pointer_cast<PlazmaZone>(strip.zone[0])->dv =
             std::abs(input->v1 - std::static_pointer_cast<PlazmaZone>(strip.zone[0])->v + 7)/8;
          std::static_pointer_cast<PlazmaZone>(strip.zone[0])->ds =
             std::abs(input->s1 - std::static_pointer_cast<PlazmaZone>(strip.zone[0])->s + 7)/8;
        }

    case HTTPD_METHOD_GET:
        httpdStartResponse(connData, 200);
        httpdHeader(connData, "Access-Control-Allow-Origin", "*"); //< allow cross-domain requests
        httpdEndHeaders(connData);
        BlockVars var = { Ws2811::xGammaLowBits, strip_drv_delay,
                          PlazmaZone::c0.r, PlazmaZone::c0.g, PlazmaZone::c0.b,
                          std::static_pointer_cast<PlazmaZone>(strip.zone[0])->v1,
                          std::static_pointer_cast<PlazmaZone>(strip.zone[0])->s1,
                          std::static_pointer_cast<PlazmaZone>(strip.zone[0])->begin,
                          std::static_pointer_cast<PlazmaZone>(strip.zone[0])->end,
                          std::static_pointer_cast<PlazmaZone>(strip.zone[0])->v,
                          std::static_pointer_cast<PlazmaZone>(strip.zone[0])->s, };
        httpdSend(connData, reinterpret_cast<const char*>(&var), sizeof var);
  }
  return HTTPD_CGI_DONE;
}

HttpdBuiltInUrl builtInUrls[]={
    {"/", cgiRedirect, "/index.html"},
    {"/flash/", cgiRedirect, "/flash/index.html"},
    {"/flash/next", cgiGetFirmwareNext, &uploadParams},
    {"/flash/upload", cgiUploadFirmware, &uploadParams},
    {"/flash/reboot", cgiRebootFirmware, 0},
    {"/ctl/strip", cgiStripControl, 0},
    {"*", cgiEspFsHook, 0}, //Catch-all cgi function for the filesystem
    {0, 0, 0}
};

void http_init(void)
{
    httpdInit(builtInUrls, 80);
}
