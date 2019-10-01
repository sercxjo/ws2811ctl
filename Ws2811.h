#ifndef WS2811_h
#define WS2811_h

#include <common_macros.h>
#include <vector>
#include <memory>
#include <algorithm>

struct Ws2811 {
  struct Color {
    uint32_t r, g, b;
    Color operator+(const Color&& x) const
    {
      return {r+x.r, g+x.g, b+x.b};
    }
    Color operator*(unsigned x) const
    {
      return {r*x, g*x, b*x};
    }
    Color operator/(unsigned x) const
    {
      return {r/x, g/x, b/x};
    }
  };
  struct Gamma2 {
    /// Translate 10-bit color value to high 8-bit color and save low 12-bit for addition to next transform
    /// High bits of input value can be used to generate periodical symmetric triangle form
    enum { INPUT_MAX=1023u };
    unsigned int lowBits:12;
    uint8_t operator()(uint16_t c)
    {
      uint32_t i= c;
      i= i&INPUT_MAX+1? INPUT_MAX+1-(i&INPUT_MAX) : 1+(i&INPUT_MAX);
      i*= i;
      --i;
      i+= lowBits;
      if (i >= (1<<20)) i= (1<<20)-1;
      lowBits= i & (1<<12)-1;
      return i>>12;
    }
    Gamma2(): lowBits(0) {}
  };
  struct Zone {
    unsigned begin:12;
    unsigned end:12; /// end - begin = size
    unsigned z:8; /// 0 is background
    /// Get color for the pixel. Must be called sequentially from 0 to size-1
    virtual Color color(uint16_t localOffset, Color backGround={0,0,0})
    {
      return backGround;
    }

    bool operator< (const Zone& b)
    {
      return z < b.z || z == b.z && begin < b.begin;
    }

    Zone(uint16_t _o, uint16_t _s, uint8_t _z=0): begin(_o), end(_o+_s), z(_z) {}
  };

  uint8_t gpio;
  std::vector<std::shared_ptr<Zone>> zone;

  Ws2811(uint8_t _gpio): gpio(_gpio)
  {}

  void start() {
  }
  void stop() {
  }
  ~Ws2811()
  {
    stop();
  }
  void send_strip() IRAM;

  static bool xGammaLowBits;  //< not accumulate low bits but preset depenantly of coordinate
};

inline Ws2811::Color operator*(unsigned x, const Ws2811::Color&& y)
{
  return y*x;
}

struct PlazmaZone: Ws2811::Zone {
  uint16_t h0;
  unsigned v:11, v1:12;
  unsigned s:11, s1:12;
  unsigned dv:8;
  unsigned ds:8;
  uint16_t h;
  char t;
  static Ws2811::Color c0;
  virtual Ws2811::Color color(uint16_t x, Ws2811::Color backGround={0,0,0}) override;
  PlazmaZone(int _o, int _s): Ws2811::Zone(_o, _s), h0(15), v(0), v1(128), s(0), s1(900), dv(16), ds(112), t(2) {}
};

extern Ws2811 strip; //< strip be defined in application
extern unsigned short strip_drv_delay;
#endif
