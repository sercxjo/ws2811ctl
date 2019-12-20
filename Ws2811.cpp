#include <Ws2811.h>
#include <espressif/esp_common.h> // sdk_os_delay_us
#include <FreeRTOS.h>
#include <esp/gpio.h>
#include <task.h>

Ws2811::Gamma2 gammaR, gammaG, gammaB;
bool Ws2811::xGammaLowBits = true;

static uint32_t getColor( uint16_t x,
  std::vector<std::shared_ptr<Ws2811::Zone>>::iterator &zb,
  std::vector<std::shared_ptr<Ws2811::Zone>>::iterator ze )
{
    if (Ws2811::xGammaLowBits) {
      gammaG.lowBits= x&2? x&1? 3<<10 : 1<<10 : x&1? 2<<10 : 0<<10;
      gammaR.lowBits= x&2? x&1? 0<<10 : 2<<10 : x&1? 1<<10 : 3<<10;
      gammaB.lowBits= x&2? x&1? 0<<10 : 2<<10 : x&1? 1<<10 : 3<<10;
    }

    while ((*zb)->end <= x && ++zb != ze);
    Ws2811::Color c= {0,0,0};
    for (auto z=zb; z != ze; z++) {
        if ((*z)->begin <= x && x < (*z)->end) c = (*z)->color( x - (*z)->begin , c );
    }
    // BRG order
    return gammaB(c.b)<<16 | gammaR(c.r)<<8 | gammaG(c.g);
}

void Ws2811::send_strip()
{
    auto zb = zone.begin();

    gpio_enable(gpio, GPIO_OUTPUT);
    gpio_write(gpio, 0);

    sdk_os_delay_us(4);
    uint32_t c= getColor( 0, zb, zone.end() );

    taskENTER_CRITICAL();

    for (uint16_t x= 0; zb != zone.end(); ) {
      for (uint32_t mask = 1<<23; mask; mask>>=1) { // transfer from hi to low bits
        gpio_write(gpio, 1);
        if (c & mask) {
          if (mask == 1) c= getColor( ++x, zb, zone.end() );
          for (volatile register uint8_t j = 3; j--; );
          gpio_write(gpio, 0);
        } else {
          gpio_write(gpio, 0);
          if (mask == 1) c= getColor( ++x, zb, zone.end() );
          for (volatile register uint8_t j = 3; j--; );
        }
      }
    }

    taskEXIT_CRITICAL();
}

Ws2811::Color PlazmaZone::c0= {1023, 923, 800};
Ws2811::Color PlazmaZone::color(uint16_t x, Ws2811::Color)
{
  if (!x) {
    h= h0;
    if (unsigned(v+dv) <= v1) v+= dv;
    else if (v < v1) v++;
    else if (v >= unsigned(v1+dv)) v-= dv;
    else if (v > v1) v--;

    if (unsigned(s+ds) <= s1) s+= ds;
    else if (s < s1) s++;
    else if (s >= unsigned(s1+ds)) s-= ds;
    else if (s > s1) s--;

    if (!--t) {
      h0+= dhdt;
      t= 3;
    }
  }
  Ws2811::Color c;
  static uint16_t H[]= {0, 11362, 31087u, 36708u, 44271u, 49893u};
  if (h < H[3])
    if (h < H[1]) { // carmine->orange
      c.r= 1023;
      c.g= h*511ul/H[1];
      c.b= 511 - c.g;
    } else if (h < H[2]){ // orange->yellow-green
      c.b= 0;
      c.g= 511u + (h-H[1])*512ul/(H[2]-H[1]);
      c.r= 1023 + 511 - c.g;
    } else { // yellow-green->emerald
      c.g= 1023;
      c.b= (h-H[2])*511ul/(H[3]-H[2]);
      c.r= 511 - c.b;
    }
  else if (h < H[4]) { // emerald->sky
      c.r= 0;
      c.b= 511u + (h-H[3])*512ul/(H[4]-H[3]);
      c.g= 1023 + 511 - c.b;
    } else if (h < H[5]) { // sky->ultramarine
      c.b= 1023;
      c.r= (h-H[4])*511ul/(H[5]-H[4]);
      c.g= 511 - c.r;
    } else { // ultramarine->carmine
      c.g= 0;
      c.r= 511u + (h-H[5])*512ul/(65536u-H[5]);
      c.b= 1023 + 511 - c.r;
    }
  c = (c0*(1024-s) + c*s)/1024 * v  /  1024;
  if (c.r<1024 && c.g<1024 && c.b<1024) h+= dhdx;
  else h-= 32700;
  return c;
}
