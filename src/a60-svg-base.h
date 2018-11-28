/// -*- mode: C++ -*-
/// Copyright (C) 2014-2018 Benjamin De Kosnik <b.dekosnik@gmail.com>

#ifndef MiL_SVG_H
#define MiL_SVG_H 1

#include <map>
#include <ostream>
#include <fstream>
#include <random>
#include <cstdint>


namespace constants
{
  // Formatting.
  constexpr char space(' ');
  constexpr char quote('"');
  constexpr char hypen('-');
  constexpr char tab('\t');
  constexpr char newline('\n');
  constexpr char comma(',');
}

namespace k = constants;

/// Utility function, like regex_replace.
inline void
string_replace(std::string& target, const std::string& match,
	       const std::string& replace)
{
  size_t pos = 0;
  while((pos = target.find(match, pos)) != std::string::npos)
    {
      target.replace(pos, match.length(), replace);
      pos += replace.length();
    }
}


/// Scalable Vector Graphics namespace
namespace svg
{
  //  Using size_type = std::size_t;
  using size_type = int;

  using std::string;

  /// Measurement abstraction type, conversion function.
  enum class unit
  {
    centimeter,		// cm
    millimeter,		// mm
    inch,		// in
    pixel		// px, 1 pixel = .264583 mm
  };

  const std::string
  to_string(const unit e)
  {
    using enum_map_type = std::map<unit, std::string>;

    static enum_map_type enum_map;
    if (enum_map.empty())
      {
	enum_map[unit::centimeter] = "cm";
	enum_map[unit::millimeter] = "mm";
	enum_map[unit::inch] = "in";
	enum_map[unit::pixel] = "px";
      }
    return enum_map[e];
  }


  /*
    HUE

    colore = color enum (and name string conversion)
    colorq = color quantified, similar to Scalar in OpenCV.

    colors = color spectrum as array
  */

  /// Color name, conversion function.
  enum class colore
  {
    white,
    gray10,
    gray20,
    gray25,		// gainsboro
    gray30,
    gray40,
    gray50,
    gray60,
    gray75,		// slategray
    gray70,
    gray80,
    gray90,
    black,

    // 06 yellow
    kanzoiro,		// daylily light orange
    kohakuiro,		// amber
    kinsusutake,        // golden-gray bamoo
    daylily,
    goldenyellow,
    hellayellow,

    // 6 yellow overflow
    antiquewhite,
    lemonchiffon,
    goldenrod,
    navajowhite,
    ivory,
    gold,


    // 12 red
    foreigncrimson,	// red
    ginshu,		// gray red
    akabeni,		// pure crimson
    akebonoiro,		// dawn color
    redorange,
    ochre,
    sohi,
    benikaba,		// red birch
    benitobi,		// red kite bird
    ake,		// scarlet/blood
    red,
    asamaorange,

    // 5 red overflow
    crimson,
    tomato,
    coral,
    salmon,
    orangered,


    // 11 green
    byakuroku,		// whitish green
    usumoegi,		// pale onion
    moegi,		// onion green
    hiwamoegi,		// siskin sprout
    midori,
    rokusho,
    aotakeiro,		// green bamboo
    seiheki,		// blue green
    seijiiro,		// celadon
    yanagizome,		// willow dye
    green,

    // 5 green overflow
    chartreuse,
    greenyellow,
    limegreen,
    springgreen,
    aquamarine,

    // 11 blue
    ultramarine,
    shinbashiiro,
    hanada,		// blue silk
    ruriiro,		// lapis
    bellflower,
    navy,
    asagiiro,		// light blue
    indigo,
    rurikon,		// dark blue lapis
    blue,
    asamablue,

    // 16 blue overflow
    cyan,
    lightcyan,
    powderblue,
    steelblue,
    cornflowerblue,
    deepskyblue,
    dodgerblue,
    lightblue,
    skyblue,
    lightskyblue,
    midnightblue,
    mediumblue,
    royalblue,
    darkslateblue,
    slateblue,
    azure,


    // 11 purple
    wisteria,
    murasaki,		// purple
    ayameiro,
    peony,
    futaai,		// dark indigo
    benimidori,		// stained red/violet
    redwisteria,	// dusty rose
    botan,		// tree peony
    kokimurasaki,	// deep purple
    usuiro,		// thin
    asamapink,

    // 10 purple overflow
    blueviolet,
    darkmagenta,
    darkviolet,
    thistle,
    plum,
    violet,
    magenta,
    deeppink,
    hotpink,
    pink,

    // 10 purple over overflow
    palevioletred,
    mediumvioletred,
    lavender,
    orchid,
    mediumorchid,
    darkestmagenta,
    mediumpurple,
    purple,
    dustyrose,
    atmosphericp,

    // STOS
    command,
    science,
    engineering,

    kissmepink,

    none
  };

  const std::string
  to_string(const colore e)
  {
    using enum_map_type = std::map<colore, std::string>;

    static enum_map_type enum_map;
    if (enum_map.empty())
      {
	enum_map[colore::white] = "rgb(255, 255, 255)";
	enum_map[colore::black] = "rgb(0, 0, 0)";
	enum_map[colore::gray90] = "rgb(25, 25, 25)";
	enum_map[colore::gray80] = "rgb(50, 50, 50)";
	enum_map[colore::gray75] = "rgb(64, 64, 64)";
	enum_map[colore::gray70] = "rgb(77, 77, 77)";
	enum_map[colore::gray60] = "rgb(100, 100, 100)";
	enum_map[colore::gray50] = "rgb(128, 128, 128)";
	enum_map[colore::gray40] = "rgb(150, 150, 150)";
	enum_map[colore::gray30] = "rgb(180, 180, 180)";
	enum_map[colore::gray25] = "rgb(191, 191, 191)";
	enum_map[colore::gray20] = "rgb(200, 200, 200)";
	enum_map[colore::gray10] = "rgb(230, 230, 230)";

	enum_map[colore::command] = "rgb(255, 0, 171)";
	enum_map[colore::science] = "rgb(150, 230, 191)";
	enum_map[colore::engineering] = "rgb(161, 158, 178)";

	enum_map[colore::kissmepink] = "rgb(255, 59, 241)";

	enum_map[colore::red] = "rgb(255, 0, 0)";
	enum_map[colore::green] = "rgb(0, 255, 0)";
	enum_map[colore::blue] = "rgb(0, 0, 255)";

	enum_map[colore::asamablue] = "rgb(1, 137, 255)";
	enum_map[colore::asamaorange] = "rgb(236, 75, 37)";
	enum_map[colore::asamapink] = "rgb(200, 56, 81)";

	// Reds
	enum_map[colore::foreigncrimson] = "rgb(201, 31, 55)";
	enum_map[colore::ginshu] = "rgb(188, 45, 41)";
	enum_map[colore::akabeni] = "rgb(195, 39,43)";
	enum_map[colore::akebonoiro] = "rgb(250, 123, 98)";
	enum_map[colore::redorange] = "rgb(220, 48, 35)";
	enum_map[colore::ochre] = "rgb(255, 78, 32)";
	enum_map[colore::sohi] = "rgb(227, 92, 56)";
	enum_map[colore::benikaba] = "rgb(157, 43, 34)";
	enum_map[colore::benitobi] = "rgb(145, 50, 40)";
	enum_map[colore::ake] = "rgb(207, 58, 36)";

	enum_map[colore::crimson] = "rgb(220, 20, 60)";
	enum_map[colore::tomato] = "rgb(255, 99, 71)";
	enum_map[colore::coral] = "rgb(255, 127, 80)";
	enum_map[colore::salmon] = "rgb(250, 128, 114)";
	enum_map[colore::orangered] = "rgb(255, 69, 0)";

	// Yellows
	enum_map[colore::kanzoiro] = "rgb(255, 137, 54)";
	enum_map[colore::kohakuiro] = "rgb(202, 105, 36)";
	enum_map[colore::kinsusutake] = "rgb(125, 78, 45)";
	enum_map[colore::daylily] = "rgb(255, 137, 54)";
	enum_map[colore::goldenyellow] = "rgb(255, 164, 0)";
	enum_map[colore::hellayellow] = "rgb(255, 255, 0)";
	enum_map[colore::antiquewhite] = "rgb(250, 235, 215)";
	enum_map[colore::lemonchiffon] = "rgb(255, 250, 205)";
	enum_map[colore::goldenrod] = "rgb(250, 250, 210)";
	enum_map[colore::navajowhite] = "rgb(255, 222, 173)";

	enum_map[colore::ivory] = "rgb(255, 255, 240)";
	enum_map[colore::gold] = "rgb(255, 215, 0)";

	// Greens
	enum_map[colore::byakuroku] = "rgb(165, 186, 147)";
	enum_map[colore::usumoegi] = "rgb(141, 178, 85)";
	enum_map[colore::moegi] = "rgb(91, 137, 48)";
	enum_map[colore::hiwamoegi] = "rgb(122, 148, 46)";
	enum_map[colore::midori] = "rgb(42, 96, 59)";
	enum_map[colore::rokusho] = "rgb(64, 122, 82)";
	enum_map[colore::aotakeiro] = "rgb(0, 100, 66)";
	enum_map[colore::seiheki] = "rgb(58, 105, 96)";
	enum_map[colore::seijiiro] = "rgb(129, 156, 139)";
	enum_map[colore::yanagizome] = "rgb(140, 158, 94)";

	enum_map[colore::chartreuse] = "rgb(127, 255, 0)";
	enum_map[colore::greenyellow] = "rgb(173, 255, 47)";
	enum_map[colore::limegreen] = "rgb(50, 205, 50)";
	enum_map[colore::springgreen] = "rgb(0, 255, 127)";
	enum_map[colore::aquamarine] = "rgb(127, 255, 212)";

	// Blues
	enum_map[colore::ultramarine] = "rgb(93, 140, 174)";
	enum_map[colore::shinbashiiro] = "rgb(0, 108, 127)";
	enum_map[colore::hanada] = "rgb(4, 79, 103)";
	enum_map[colore::ruriiro] = "rgb(31, 71, 136)";
	enum_map[colore::bellflower] = "rgb(25, 31, 69)";
	enum_map[colore::navy] = "rgb(0, 49, 113)";
	enum_map[colore::asagiiro] = "rgb(72, 146, 155)";
	enum_map[colore::indigo] = "rgb(38, 67, 72)";
	enum_map[colore::rurikon] = "rgb(27, 41, 75)";
	enum_map[colore::cyan] = "rgb(0, 255, 255)";

	enum_map[colore::lightcyan] = "rgb(224, 255, 255)";
	enum_map[colore::powderblue] = "rgb(176, 224, 230)";
	enum_map[colore::steelblue] = "rgb(70, 130, 237)";
	enum_map[colore::cornflowerblue] = "rgb(100, 149, 237)";
	enum_map[colore::deepskyblue] = "rgb(0, 191, 255)";
	enum_map[colore::dodgerblue] = "rgb(30, 144, 255)";
	enum_map[colore::lightblue] = "rgb(173, 216, 230)";
	enum_map[colore::skyblue] = "rgb(135, 206, 235)";
	enum_map[colore::lightskyblue] = "rgb(173, 206, 250)";
	enum_map[colore::midnightblue] = "rgb(25, 25, 112)";

	enum_map[colore::mediumblue] = "rgb(0, 0, 205)";
	enum_map[colore::royalblue] = "rgb(65, 105, 225)";
	enum_map[colore::darkslateblue] = "rgb(72, 61, 139)";
	enum_map[colore::slateblue] = "rgb(106, 90, 205)";
	enum_map[colore::azure] = "rgb(240, 255, 255)";

	// Purples
	enum_map[colore::wisteria] = "rgb(135, 95, 154)";
	enum_map[colore::murasaki] = "rgb(79, 40, 75)";
	enum_map[colore::ayameiro] = "rgb(118, 53, 104)";
	enum_map[colore::peony] = "rgb(164, 52, 93)";
	enum_map[colore::futaai] = "rgb(97, 78, 110)";
	enum_map[colore::benimidori] = "rgb(120, 119, 155)";
	enum_map[colore::redwisteria] = "rgb(187, 119, 150)";
	enum_map[colore::botan] = "rgb(164, 52, 93)";
	enum_map[colore::kokimurasaki] = "rgb(58, 36, 59)";
	enum_map[colore::usuiro] = "rgb(168, 124, 160)";

	enum_map[colore::blueviolet] = "rgb(138, 43, 226)";
	enum_map[colore::darkmagenta] = "rgb(139, 0, 139)";
	enum_map[colore::darkviolet] = "rgb(148, 0, 211)";
	enum_map[colore::thistle] = "rgb(216, 191, 216)";
	enum_map[colore::plum] = "rgb(221, 160, 221)";
	enum_map[colore::violet] = "rgb(238, 130, 238)";
	enum_map[colore::magenta] = "rgb(255, 0, 255)";
	enum_map[colore::deeppink] = "rgb(255, 20, 147)";
	enum_map[colore::hotpink] = "rgb(255, 105, 180)";
	enum_map[colore::pink] = "rgb(255, 192, 203)";

	enum_map[colore::palevioletred] = "rgb(219, 112, 147)";
	enum_map[colore::mediumvioletred] = "rgb(199, 21, 133)";
	enum_map[colore::lavender] = "rgb(230, 230, 250)";
	enum_map[colore::orchid] = "rgb(218, 112, 214)";
	enum_map[colore::mediumorchid] = "rgb(186, 85, 211)";
	enum_map[colore::darkestmagenta] = "rgb(180, 0, 180)";
	enum_map[colore::mediumpurple] = "rgb(147, 112, 219)";
	enum_map[colore::purple] = "rgb(128, 0, 128)";
	enum_map[colore::dustyrose] = "rgb(191, 136, 187)";
	enum_map[colore::atmosphericp] = "rgb(228, 210, 231)";

	enum_map[colore::none] = "none";
      }
    return enum_map[e];
  }


  /// Color quantified as components in aggregate type with RGB values.
  /// aka like Scalar in OpenCV.
  struct colorq
  {
    using itype = unsigned short;

    itype	r;
    itype	g;
    itype	b;

    // Return "rgb(64, 64, 64)";
    string
    static to_string(colorq s)
    {
      std::ostringstream oss;
      oss << "rgb(" << s.r << ',' << s.g << ',' << s.b << ")";
      return oss.str();
    }

    // From "rgb(64, 64, 64)";
    colorq
    static from_string(string s)
    {
      // Kill rgb() enclosing.
      s.pop_back();
      s = s.substr(4);

      // String stream which eats whitespace and knows number separation.
      std::istringstream iss(s);
      iss >> std::skipws;

      char c(0);

      ushort rs(0);
      iss >> rs;
      itype r = static_cast<itype>(rs);
      iss >> c;

      ushort gs(0);
      iss >> gs;
      itype g = static_cast<itype>(gs);
      iss >> c;

      ushort bs(0);
      iss >> bs;
      itype b = static_cast<itype>(bs);

      return colorq(r, g, b);
    }

    colorq() = default;
    colorq(const colorq&) = default;
    colorq& operator=(const colorq&) = default;

    colorq(itype ra, itype ga, itype ba) : r(ra), g(ga), b(ba) { }

    colorq(const colore e)
    {
      colorq klr = from_string(svg::to_string(e));
      r = klr.r;
      b = klr.b;
      g = klr.g;
    }
  };


  /*
    Combine color a with color b in percentages ad and ab, respectively.

    To average, constrain paramters ad and ab such that: ad + ab == 2.

    Like so:
    ushort ur = (a.r + b.r) / 2;
    ushort ug = (a.g + b.g) / 2;
    ushort ub = (a.b + b.b) / 2;

  */
  colorq
  combine_two_colorq(const colorq& a, double ad, const colorq& b, double bd)
  {
    using itype = colorq::itype;

    auto denom = ad + bd;
    auto ur = ((a.r * ad) + (b.r * bd)) / denom;
    auto ug = ((a.g * ad) + (b.g * bd)) / denom;
    auto ub = ((a.b * ad) + (b.b * bd)) / denom;
    itype cr = static_cast<itype>(ur);
    itype cg = static_cast<itype>(ug);
    itype cb = static_cast<itype>(ub);
    return colorq { cr, cg, cb };
  }

  colorq
  average_two_colorq(const colorq& a, const colorq& b)
  { return combine_two_colorq(a, 1.0, b, 1.0); }


  /// Color iteration and combinatorics.
  using colores = std::vector<colore>;
  using colorqs = std::vector<colorq>;

  using color_array = std::array<colore, 116>;


  // Color spectrum.
  color_array colors =
  {
    // black to gray to white in 10% and 25% increments
    colore::white,
    colore::gray10, colore::gray20, colore::gray25, colore::gray30,
    colore::gray40, colore::gray50, colore::gray60, colore::gray70,
    colore::gray75, colore::gray80, colore::gray90,
    colore::black,

    // yellow
    colore::hellayellow, colore::goldenyellow,
    colore::kanzoiro, colore::kohakuiro, colore::kinsusutake, colore::daylily,
    colore::antiquewhite, colore::lemonchiffon, colore::goldenrod,
    colore::navajowhite, colore::ivory, colore::gold,

    // red (orange)
    colore::red, colore::foreigncrimson, colore::ginshu, colore::akabeni,
    colore::akebonoiro, colore::redorange, colore::ochre, colore::sohi,
    colore::benikaba, colore::benitobi, colore::ake, colore::asamaorange,
    colore::crimson, colore::tomato, colore::coral, colore::salmon,
    colore::orangered,

    // green
    colore::green, colore::byakuroku, colore::usumoegi, colore::moegi,
    colore::midori, colore::rokusho, colore::aotakeiro, colore::seiheki,
    colore::seijiiro, colore::yanagizome, colore::hiwamoegi,
    colore::chartreuse, colore::greenyellow, colore::limegreen,
    colore::springgreen, colore::aquamarine,

    // blue
    colore::ultramarine, colore::shinbashiiro, colore::hanada, colore::ruriiro,
    colore::bellflower, colore::navy, colore::asagiiro, colore::indigo,
    colore::rurikon, colore::blue, colore::asamablue,
    colore::cyan, colore::lightcyan, colore::powderblue, colore::steelblue,
    colore::cornflowerblue, colore::deepskyblue, colore::dodgerblue,
    colore::lightblue, colore::skyblue, colore::lightskyblue,
    colore::midnightblue, colore::mediumblue, colore::royalblue,
    colore::darkslateblue, colore::slateblue,
    colore::azure,

    // purple (magenta, violet, pink)
    colore::wisteria, colore::murasaki, colore::ayameiro, colore::peony,
    colore::futaai, colore::benimidori, colore::redwisteria, colore::botan,
    colore::kokimurasaki, colore::usuiro, colore::asamapink,
    colore::blueviolet, colore::darkmagenta, colore::darkviolet,
    colore::thistle, colore::plum, colore::violet, colore::magenta,
    colore::deeppink, colore::hotpink, colore::pink,
    colore::palevioletred, colore::mediumvioletred, colore::lavender,
    colore::orchid, colore::mediumorchid, colore::darkestmagenta,
    colore::mediumpurple, colore::purple, colore::dustyrose,
    colore::atmosphericp
  };


  // Loop through color array starting at position c.
  colore
  next_color(colore c)
  {
    colore cnext = colors.front();
    auto itr = std::find(colors.begin(), colors.end(), c);
    if (itr != colors.end())
      {
	++itr;
	if (itr == colors.end())
	  itr = colors.begin();
	cnext = *itr;
      }
    return cnext;
  }


  // Specific regions of spectrum as bands of color.
  // Each band has a starting hue and a number of known good samples.
  // This is then used to seed a generator that computes more of similar hues.
  using colorband = std::tuple<colore, ushort>;
  constexpr colorband cband_bw = std::make_tuple(colore::white, 12);
  constexpr colorband cband_y = std::make_tuple(colore::hellayellow, 11);
  constexpr colorband cband_r = std::make_tuple(colore::red, 16);
  constexpr colorband cband_g = std::make_tuple(colore::green, 15);
  constexpr colorband cband_b = std::make_tuple(colore::ultramarine, 26);
  constexpr colorband cband_p = std::make_tuple(colore::wisteria, 30);


  // Add white to tint in r ammount.
  colorq
  tint_to(const colorq c, const double density)
  {
    colorq klr;
    klr.r = c.r + (255 - c.r) * density;
    klr.g = c.g + (255 - c.g) * density;
    klr.b = c.b + (255 - c.b) * density;
    return klr;
  }

  // Add black to shade in r ammount.
  colorq
  shade_to(const colorq c, const double density)
  {
    colorq klr;
    klr.r = c.r * (1.0 - density);
    klr.g = c.r * (1.0 - density);
    klr.b = c.b * (1.0 - density);
    return klr;
  }

  /*
    Generate a color band from starting hue and seeds.

    Algorithm is average two known good, where two picked randomly.

    Return type is a vector of generated colorq types.
   */
  colorqs
  color_band_from(const colorband& cb, const ushort neededh)
  {
    // Find starting hue and number of samples in the color band.
    colore c = std::get<0>(cb);
    ushort hn = std::get<1>(cb);

    // Find initial offset.
    auto itr = std::find(colors.begin(), colors.end(), c);
    if (itr == colors.end())
      {
	string m("collection::color_band_from: " + to_string(c));
	throw std::runtime_error(m);
      }

    // Randomness.
    static std::mt19937_64 rg(std::random_device{}());

    // Setup random picker of sample hues in band.
    auto disti = std::uniform_int_distribution<>(0, hn);

    // Set up random percentage for tint or fade or mix.
#if 0
    auto distr = std::uniform_real_distribution<>(0.3, 0.7);
    auto distb = std::uniform_int_distribution<>(0, 1);
    auto ftint = &tint_to;
    auto fshade = &shade_to;
#endif

    // Generate new from averaging random samples, cache in return vector.
    colorqs cband;
    for (ushort i = 0; i < neededh; ++i)
      {
	// New color.
	ushort o1 = disti(rg);
	ushort o2 = disti(rg);
	colore c1 = *(itr + o1);
	colore c2 = *(itr + o2);
	colorq cgen = average_two_colorq(c1, c2);

#if 0
	// Add tint or shade.
	// XXX
	auto fklr = distb(rg) ? ftint : fshade;
	colorq cgent = (*fklr)(cgen, distr(rg));
#else
	colorq cgent = cgen;
#endif

	cband.push_back(cgent);
      }

    return cband;
  }

  colorq
  next_in_color_band(const colorband& cb)
  {
    // Generate bands.
    // For now, just generate 100 each.
    const ushort bandn = 100;
    static colorqs gband_bw = color_band_from(cband_bw, bandn);
    static colorqs gband_y = color_band_from(cband_y, bandn);
    static colorqs gband_r = color_band_from(cband_r, bandn);
    static colorqs gband_g = color_band_from(cband_g, bandn);
    static colorqs gband_b = color_band_from(cband_b, bandn);
    static colorqs gband_p = color_band_from(cband_p, bandn);

    // XXX No out of bounds error checking.
    colorq ret;
    const colore c = std::get<0>(cb);
    switch (c)
      {
      case colore::white:
	ret = gband_bw.back();
	gband_bw.pop_back();
	break;
      case colore::hellayellow:
	ret = gband_y.back();
	gband_y.pop_back();
	break;
      case colore::red:
	ret = gband_r.back();
	gband_r.pop_back();
	break;
      case colore::green:
	ret = gband_g.back();
	gband_g.pop_back();
	break;
      case colore::ultramarine:
	ret = gband_b.back();
	gband_b.pop_back();
	break;
      case colore::wisteria:
	ret = gband_p.back();
	gband_p.pop_back();
	break;
      default:
	throw std::runtime_error("collection::next_in_color_band");
	break;
      }
    return ret;
  }


  /**
     Area/Page/Canvas/Drawing area description.
     Size, origin location in 2D (x,y), heigh, width

     ANSI Letter mm == (unit::millimeter, 215.9, 279.4);
     ANSI Letter pixels == (unit::pixel, 765, 990);

     ISO A4 mm == (unit::millimeter, 210, 297);
     ISO A4 pixels == (unit::pixel, 744.09, 1052.36);

     720p pixels == (unit::pixel, 1280, 720);
     1080p pixels == (unit::pixel, 1920, 1080);

     SVG coordinate system is (0,0) is top leftmost.
  */
  template<typename T1 = svg::size_type>
  struct area
  {
    using size_type = T1;
    using unit = svg::unit;

    unit		_M_unit;
    size_type		_M_width;
    size_type		_M_height;
  };


  /// Datum consolidating style preferences.
  struct style
  {
    using size_type = svg::size_type;
    using colorq = svg::colorq;

    colorq		_M_fill_color;
    double		_M_fill_opacity;

    colorq		_M_stroke_color;
    double		_M_stroke_opacity;
    double		_M_stroke_size;

    static string
    str(const style& s)
    {
      const string space("; ");
      std::ostringstream stream;
      stream << k::space;
      stream << "style=" << k::quote;
      stream << "fill:" << colorq::to_string(s._M_fill_color) << space;
      stream << "fill-opacity:" << s._M_fill_opacity << space;
      stream << "stroke:" << colorq::to_string(s._M_stroke_color) << space;
      stream << "stroke-opacity:" << s._M_stroke_opacity << space;
      stream << "stroke-width:" << s._M_stroke_size << k::quote;
      return stream.str();
    }
  };


  /// Datum consolidating filter preferences.
  struct filter
  {
    using size_type = svg::size_type;

    static string
    str(const size_type n = 10)
    {
      const string filtername("gblur");

      std::ostringstream stream;
      stream << k::space;
      stream << "filter=" << k::quote;
      stream << "url(#" << filtername << n << ")" << k::quote;
      return stream.str();
    }

    static string
    str(const string s)
    {
      const string filtername("gblur");

      std::ostringstream stream;
      stream << k::space;
      stream << "filter=" << k::quote;
      stream << "url(#" << filtername << s << ")" << k::quote;
      return stream.str();
    }
  };

  /// Datum consolidating transform preferences.
  struct transform
  {
    static string
    rotate(int deg, int x, int y)
    {
      std::ostringstream stream;
      stream << k::space;
      stream << "transform=" << k::quote;
      stream << "rotate(" << std::to_string(deg)
	     << k::space << x << k::space << y
	     << ")" << k::quote;
      return stream.str();
    }
  };


  /**
     Character rendering, type, fonts, styles.

     Expect to keep changing the output, so use this abstraction to set
     styling defaults, so that one can just assign types instead of doing
     a bunch of search-and-replace operations when changing type
     characteristics.

     SVG Fonts
     https://developer.mozilla.org/en-US/docs/Web/SVG/Tutorial/SVG_fonts
  */
  struct typography
  {
    using size_type = svg::size_type;

    // Text alignment.
    enum class align { left, center, right, justify, justifyall,
		       start, end, inherit, matchparent, initial, unset };

    // How to align text to a given point.
    // middle == center the middle of the text block at point.
    // start  == start the text block at point.
    // end    == end the text block at point.
    enum class anchor { start, middle, end, inherit };

    // Light is 300, Normal is 400, and Medium is 500 in CSS.
    enum class weight { xlight, light, normal, medium, bold, xbold };

    // Face variant.
    enum class property { normal, italic };

    // Find installed fonts on linux with `fc-list`
    std::string		_M_face;	// System font name
    size_type		_M_size;	// Display size
    style		_M_style;

    // Style attributes for text
    // font-weight, transform,
    // https://developer.mozilla.org/en-US/docs/Web/SVG/Element/font-face
    anchor		_M_a;
    align		_M_align;
    weight		_M_w;
    property		_M_p;

    const std::string
    to_string(const align a) const
    {
      using enum_map_type = std::map<align, std::string>;

      static enum_map_type enum_map;
      if (enum_map.empty())
	{
	  enum_map[align::left] = "left";
	  enum_map[align::center] = "center";
	  enum_map[align::right] = "right";
	  enum_map[align::justify] = "justify";
	  enum_map[align::justifyall] = "justify-all";
	  enum_map[align::start] = "start";
	  enum_map[align::end] = "end";
	  enum_map[align::inherit] = "inherit";
	  enum_map[align::matchparent] = "match-parent";
	  enum_map[align::initial] = "initial";
	  enum_map[align::unset] = "unset";
	}
      return enum_map[a];
    }

    const std::string
    to_string(const anchor a) const
    {
      using enum_map_type = std::map<anchor, std::string>;

      static enum_map_type enum_map;
      if (enum_map.empty())
	{
	  enum_map[anchor::start] = "start";
	  enum_map[anchor::middle] = "middle";
	  enum_map[anchor::end] = "end";
	  enum_map[anchor::inherit] = "inherit";
	}
      return enum_map[a];
    }

    const std::string
    to_string(const weight w) const
    {
      using enum_map_type = std::map<weight, std::string>;

      static enum_map_type enum_map;
      if (enum_map.empty())
	{
	  enum_map[weight::xlight] = "200";
	  enum_map[weight::light] = "300";
	  enum_map[weight::normal] = "400";
	  enum_map[weight::medium] = "500";
	  enum_map[weight::bold] = "600";
	  enum_map[weight::xbold] = "700";
	}
      return enum_map[w];
    }

    const std::string
    to_string(const property p) const
    {
      using enum_map_type = std::map<property, std::string>;

      static enum_map_type enum_map;
      if (enum_map.empty())
	{
	  enum_map[property::normal] = "normal";
	  enum_map[property::italic] = "italic";
	}
      return enum_map[p];
    }

    const std::string
    add_attribute() const
    {
      const std::string name("__name");
      const std::string size("__size");
      const std::string align("__align");

      std::string strip1 = \
	R"(font-family="__name" font-size="__size" text-align="__align" )";

      string_replace(strip1, name, _M_face);
      string_replace(strip1, size, std::to_string(_M_size));
      string_replace(strip1, align, to_string(_M_align));

      const std::string weight("__weight");
      const std::string property("__prop");

      std::string strip2 = \
	R"(font-weight="__weight" font-style="__prop")";

      string_replace(strip2, weight, to_string(_M_w));
      string_replace(strip2, property, to_string(_M_p));

      return strip1 + strip2;
    }
  };
} // namespace svg


namespace constants
{
  using namespace svg;

  /// Useful area constants.
  constexpr area<float> mm_letter = { unit::millimeter, 215.9, 279.4 };
  constexpr area<float> mm_a4 = { unit::millimeter, 210, 297 };

  constexpr area<> px_letter_area = { unit::pixel, 765, 990 };
  constexpr area<> px_a4_area = { unit::pixel, 765, 990 };
  constexpr area<> px_720p_area = { unit::pixel, 1280, 720 };
  constexpr area<> px_1080p_area = { unit::pixel, 1920, 1080 };


  /// Style constants.
  const colorq whiteq(colore::white);
  const colorq blackq(colore::black);
  const style w_style = { whiteq, 1.0, blackq, 0.0, 0 };
  const style b_style = { blackq, 1.0, whiteq, 0.0, 0 };

  /// Useful typography constants.
  typography::anchor a = typography::anchor::middle;
  typography::align al = typography::align::center;
  typography::weight w = typography::weight::normal;
  typography::property p = typography::property::normal;

  const char* sserif = "Source Serif Pro";
  const char* ssans = "Source Sans Pro";
  const char* smono = "Source Code Pro";
  const char* shsans = "Source Han Sans CN";
  const typography smono_typo = { smono, 12, b_style, a, al, w, p };
  const typography sserif_typo = { sserif, 12, b_style, a, al, w, p };
  const typography ssans_typo = { ssans, 12, b_style, a, al, w, p };
  const typography shsans_typo = { shsans, 12, b_style, a, al, w, p };
  const typography apercu_typo = { "Apercu", 4, b_style, a, al, w, p };

  // Default typography.
  //
  // NB: Using a global locale. So, filenames are in multiple
  // languages, not just en_US.
  //
  // Make sure that the font with the largest support for random
  // characters is used, ie some variant of Source Sans, Source Han
  // Sans XX, etc. Otherwise, blank spaces in rendered output are likely.
  const typography d_typo = shsans_typo;
};


namespace svg
{
/// The base type for all svg elements.
struct element_base
{
  using size_type = svg::size_type;
  using stream_type = std::ostringstream;
  using style = svg::style;
  using filter = svg::filter;

  stream_type		_M_sstream;

  virtual void
  start_element() = 0;

  virtual void
  finish_element() = 0;

  // Empty when the output buffer is.
  bool
  empty() { return _M_sstream.str().empty(); }

  string
  str() { return _M_sstream.str(); }

  void
  str(const string& s) { return _M_sstream.str(s); }

  void
  add_filter(const size_type n)
  { _M_sstream << filter::str(n); }

  void
  add_filter(const string s)
  { _M_sstream << filter::str(s); }

  void
  add_style(const style& sty)
  { _M_sstream << style::str(sty); }

  // Common transforms include rotate(180)
  void
  add_transform(const string s)
  { _M_sstream << k::space << "transform=" << k::quote << s << k::quote; }
};


/*
  Text SVG element.

  Specification reference:
  https://developer.mozilla.org/en-US/docs/Web/SVG/Element/text

  Attributes:
  x, y, dx, dy, text-anchor, rotate, textLength, lengthAdjust
 */
struct text_form : virtual public element_base
{
  using size_type = svg::size_type;
  using colore = svg::colore;
  using style = svg::style;
  using typography = svg::typography;

  struct data
  {
    size_type		_M_x_origin;
    size_type		_M_y_origin;
    std::string		_M_text;
    typography		_M_typo;
  };

  // Either serialize immediately (as below), or create data structure
  // that adds data to data_vec and then finish_element serializes.
  void
  add_data(const data& d, string transform = " ")
  {
    const std::string x("__x");
    const std::string y("__y");
    const std::string anchor("__anchor");
    const std::string attr("__attr");
    const std::string style("__style");
    const std::string trans("__trans");

    std::string strip = R"_delimiter_(x="__x" y="__y" text-anchor="__anchor"  __attr __style __trans>)_delimiter_";

    string_replace(strip, x, std::to_string(d._M_x_origin));
    string_replace(strip, y, std::to_string(d._M_y_origin));
    string_replace(strip, anchor, d._M_typo.to_string(d._M_typo._M_a));
    string_replace(strip, attr, d._M_typo.add_attribute());
    string_replace(strip, style, svg::style::str(d._M_typo._M_style));
    string_replace(strip, trans, transform);
    _M_sstream << strip;
    _M_sstream << d._M_text;
  }

  void
  start_element()
  { _M_sstream << "<text "; }

  void
  finish_element();

  // For text list output, use tspan for line breaks.  This span
  // creates a new horizontal line for every tspan block, starting at
  // xpos with spacing dy (1.2em).
  static string
  start_tspan_y(uint xpos, string dy)
  {
    const std::string x("__x");
    const std::string dys("__dy");
    std::string strip = R"_delimiter_(<tspan x="__x" dy="__dy">)_delimiter_";
    string_replace(strip, x, std::to_string(xpos));
    string_replace(strip, dys, dy);
    return strip;
  }

  static string
  start_tspan_y(uint xpos, uint dy)
  { return start_tspan_y(xpos, std::to_string(dy)); }

  // For text list output, use tspan for line breaks.  This span
  // creates a new vertical line space for every tspan block, starting
  // at xpos with horizontal spacing dx ("1.4em").
  static string
  start_tspan_x(uint xpos, string dx)
  {
    const std::string x("__x");
    const std::string dxs("__dx");
    std::string strip = R"_delimiter_(<tspan x="__x" dx="__dx">)_delimiter_";
    string_replace(strip, x, std::to_string(xpos));
    string_replace(strip, dxs, dx);
    return strip;
  }

  static string
  start_tspan_x(uint xpos, uint dx)
  { return start_tspan_x(xpos, std::to_string(dx)); }

  static string
  finish_tspan()
  { return "</tspan>"; }
};

void
text_form::finish_element()
{ _M_sstream  << "</text>" << std::endl; }

string
make_tspan_y_from_string_by_token(string s, uint xpos, const char token = ' ')
{
  string start(text_form::start_tspan_y(xpos, "0.5em"));
  string ret = start;
  for (uint i = 0; i < s.size(); ++i)
    {
      const char c = s[i];
      if (c != token)
	ret += c;
      else
	{
	  ret += text_form::finish_tspan();
	  if (i < s.size() - 1)
	    ret += start;
	}
    }
  ret += text_form::finish_tspan();
  return ret;
}


double
pt_to_px(uint i)
{ return 1.25 * i; }


/*
  Rectangle SVG element.

  Specification reference:
  https://developer.mozilla.org/en-US/docs/Web/SVG/Element/rect

  Attributes:
  x, y, width, height, rx, ry
 */
struct rect_form : virtual public element_base
{
  using size_type = svg::size_type;
  using style = svg::style;

  struct data
  {
    size_type		_M_x_origin;
    size_type		_M_y_origin;
    size_type		_M_width;
    size_type		_M_height;
  };

  // Either serialize immediately (as below), or create data structure
  // that adds data to data_vec and then finish_element serializes.
  void
  add_data(const data& d)
  {
    const std::string x("__x");
    const std::string y("__y");
    const std::string w("__w");
    const std::string h("__h");

    std::string strip = R"_delimiter_(x="__x" y="__y" width="__w" height="__h"
)_delimiter_";

    string_replace(strip, x, std::to_string(d._M_x_origin));
    string_replace(strip, y, std::to_string(d._M_y_origin));
    string_replace(strip, w, std::to_string(d._M_width));
    string_replace(strip, h, std::to_string(d._M_height));
    _M_sstream << strip;
  }

  void
  start_element()
  { _M_sstream << "<rect "; }

  void
  finish_element();
};

void
rect_form::finish_element()
{ _M_sstream  << " />" << std::endl; }


/*
  Image SVG element. This can be another SVG file, or can be a raster
  image format like PNG or JPEG.

  Specification reference:
  https://developer.mozilla.org/en-US/docs/Web/SVG/Element/image

  Attributes:
  x, y, width, height, xlink:xref, preserveAspectRatio
 */
struct image_form : virtual public element_base
{
  using size_type = svg::size_type;

  struct data
  {
    std::string		_M_xref;
    size_type		_M_x_origin;
    size_type		_M_y_origin;
    size_type		_M_width;
    size_type		_M_height;
  };

  // Either serialize immediately (as below), or create data structure
  // that adds data to data_vec and then finish_element serializes.
  void
  add_data(const data& d)
  {
    const std::string x("__x");
    const std::string y("__y");
    const std::string w("__w");
    const std::string h("__h");
    const std::string ref("__ref");

    std::string strip = R"_delimiter_(xlink:href="__ref" x="__x" y="__y" width="__w" height="__h"
)_delimiter_";

    string_replace(strip, ref, d._M_xref);
    string_replace(strip, x, std::to_string(d._M_x_origin));
    string_replace(strip, y, std::to_string(d._M_y_origin));
    string_replace(strip, w, std::to_string(d._M_width));
    string_replace(strip, h, std::to_string(d._M_height));
    _M_sstream << strip;
  }

  void
  start_element()
  { _M_sstream << "<image "; }

  void
  finish_element();
};


void
image_form::finish_element()
{ _M_sstream  << " />" << std::endl; }


/*
  Circle SVG element.

  Specification reference:
  https://developer.mozilla.org/en-US/docs/Web/SVG/Element/circle

  Attributes:
  x, y, width, height, xlink:xref, preserveAspectRatio
 */
struct circle_form : virtual public element_base
{
  using size_type = svg::size_type;
  using style = svg::style;

  struct data
  {
    size_type		_M_x_origin;
    size_type		_M_y_origin;
    size_type		_M_radius;
  };

  // Either serialize immediately (as below), or create data structure
  // that adds data to data_vec and then finish_element serializes.
  void
  add_data(const data& d, string transform = " ")
  {
    const std::string x("__x");
    const std::string y("__y");
    const std::string r("__r");
    const std::string trans("__trans");

    std::string strip = R"_delimiter_(cx="__x" cy="__y" r="__r" __trans
)_delimiter_";

    string_replace(strip, x, std::to_string(d._M_x_origin));
    string_replace(strip, y, std::to_string(d._M_y_origin));
    string_replace(strip, r, std::to_string(d._M_radius));
    string_replace(strip, trans, transform);
    _M_sstream << strip;
  }

  void
  start_element()
  { _M_sstream << "<circle "; }

  void
  finish_element();
};

void
circle_form::finish_element()
{ _M_sstream  << " />" << std::endl; }


/*
  Line SVG element.

  Specification reference:
  https://developer.mozilla.org/en-US/docs/Web/SVG/Element/line

  Attributes:
  x, y, width, height, xlink:xref, preserveAspectRatio
 */
struct line_form : virtual public element_base
{
  using size_type = svg::size_type;
  using style = svg::style;

  struct data
  {
    size_type		_M_x_begin;
    size_type		_M_x_end;
    size_type		_M_y_begin;
    size_type		_M_y_end;
  };

  // Either serialize immediately (as below), or create data structure
  // that adds data to data_vec and then finish_element serializes.
  void
  add_data(const data& d)
  {
    const std::string x1("__x1");
    const std::string x2("__x2");
    const std::string y1("__y1");
    const std::string y2("__y2");

    std::string strip = R"_delimiter_(x1="__x1" y1="__y1" x2="__x2" y2="__y2"
)_delimiter_";

    string_replace(strip, x1, std::to_string(d._M_x_begin));
    string_replace(strip, x2, std::to_string(d._M_x_end));
    string_replace(strip, y1, std::to_string(d._M_y_begin));
    string_replace(strip, y2, std::to_string(d._M_y_end));
    _M_sstream << strip;
  }

  void
  start_element()
  { _M_sstream << "<line "; }

  void
  finish_element();
};

void
line_form::finish_element()
{ _M_sstream  << " />" << std::endl; }


/*
  Group SVG element.

  Specification reference:
  https://developer.mozilla.org/en-US/docs/Web/SVG/Element/g

  Attributes:
  x, y, width, height, xlink:xref, preserveAspectRatio
 */
struct group_form : virtual public element_base
{
  void
  start_element()
  { _M_sstream << "<g>" << std::endl; }

  // For groups of elements that have the same name.
  void
  start_element(string name)
  { _M_sstream << "<g id=" << '"' << name <<'"' << ">" << std::endl; }

  // For groups of elements that have the same style.
  void
  start_element(string name, const style& sty)
  {
    _M_sstream << "<g id=" << '"' << name <<'"' << ' ';
    add_style(sty);
    _M_sstream << '>' << std::endl;
  }

  // For groups of elements that have the same transform.
  // This overload is for rotation.
  void
  start_element(const svg::transform, int deg, int x, int y)
  {
    _M_sstream << "<g " << svg::transform::rotate(deg, x, y);
    _M_sstream << '>' << std::endl;
  }

  void
  finish_element();
};

void
group_form::finish_element()
{ _M_sstream  << "</g>" << std::endl; }


/**
   SVG object
*/
struct svg_form : public element_base
{
  using area = svg::area<>;
  using typography = svg::typography;


  const std::string	_M_name;
  const area		_M_area;
  const typography&	_M_typo;

  svg_form(const std::string __title, const area& __cv = k::px_letter_area,
	   const typography& __typo = k::smono_typo)
  : _M_name(__title), _M_area(__cv), _M_typo(__typo)
  { start(); }

  svg_form(const svg_form& other)
  : _M_name(other._M_name), _M_area(other._M_area), _M_typo(other._M_typo)
  { }

  ~svg_form() { finish(); }

  void
  start_element();

  void
  finish_element();

  void
  add_title();

  void
  add_filters();

  void
  add_element(element_base& e)
  { _M_sstream << e.str(); }

  void
  write();

  void
  start()
  {
    this->start_element();
    this->add_title();
    this->add_filters();
  }

  void
  finish()
  {
    this->finish_element();
    this->write();
  }
};


// Take input size and make a one channel (single-image) SVG form.
svg_form
make_svg_1_channel(const int deltax, const int deltay, const string& outbase)
{
  using namespace svg;
  area<> a = { unit::pixel, deltax, deltay };
  return svg_form(outbase, a);
}


void
make_1_channel_insert(svg_form& obj, string insert1)
{
  if (!insert1.empty())
    {
      rect_form r1;
      r1.str(insert1);
      obj.add_element(r1);
    }
}


// Take input size and make a two channel (single-image) SVG form.
svg_form
make_svg_2_channel(const int deltax, const int deltay, const string& outbase)
{
  using namespace svg;
  area<> a = { unit::pixel, 2 * deltax, deltay };
  return svg_form(outbase, a);
}


void
make_2_channel_insert(svg_form& obj, string insert1, string insert2)
{
  if (!insert1.empty())
    {
      rect_form r1;
      r1.str(insert1);
      obj.add_element(r1);
    }

  if (!insert2.empty())
    {
      rect_form r2;
      r2.str(insert2);
      obj.add_element(r2);
    }
}


// Draws a circle around a point (x,y), of style (s), of radius (r).
void
point_2d_to_circle(svg_form& obj, double x, double y, svg::style s,
		   const int r = 4, const string transform = " ")
{
  circle_form c;
  using size_type = svg::size_type;
  size_type xi = static_cast<size_type>(std::round(x));
  size_type yi = static_cast<size_type>(std::round(y));
  circle_form::data dc = { xi, yi, r };

  c.start_element();
  c.add_data(dc, transform);
  c.add_style(s);
  c.finish_element();
  obj.add_element(c);
}


// Lines radiating from center point (x,y).
void
point_2d_to_ray(svg_form& obj, double x, double y, svg::style s,
		int r = 4, const uint nrays = 10)
{
  using size_type = svg::size_type;

  size_type xi = static_cast<size_type>(std::round(x));
  size_type yi = static_cast<size_type>(std::round(y));

  // End points on the ray.
  // Pick a random ray, use an angle in the range [0, 2pi].
  static std::mt19937_64 rg(std::random_device{}());
  auto distr = std::uniform_real_distribution<>(0.0, 2 * 22/7);
  auto disti = std::uniform_int_distribution<>(-3, 3);

  for (uint i = 0; i < nrays; ++i)
    {
      double theta = distr(rg);
      double rvary = disti(rg);

      size_type xe = xi + (r + rvary) * std::cos(theta);
      size_type ye = yi + (r + rvary) * std::sin(theta);

      line_form::data dr = { xi, xe, yi, ye };
      line_form ray;
      ray.start_element();
      ray.add_data(dr);
      ray.add_style(s);
      ray.finish_element();
      obj.add_element(ray);
    }
}


void
point_2d_to_rect(svg_form& obj, double x, double y, svg::style s,
		 int width = 4, int height = 4)
{
  rect_form r;
  using size_type = svg::size_type;
  size_type xi = static_cast<size_type>(std::round(x));
  size_type yi = static_cast<size_type>(std::round(y));
  rect_form::data dr = { xi, yi, width, height };

  r.start_element();
  r.add_data(dr);
  r.add_style(s);
  r.finish_element();
  obj.add_element(r);
}


//
// XXX This should be a .cc file.
//
void
svg_form::write()
{
  try
    {
      std::string filename(_M_name + ".svg");
      std::ofstream f(filename);
      if (!f.is_open() || !f.good())
	throw std::runtime_error("svg_form::write fail");

      f << _M_sstream.str() << std::endl;
    }
  catch(std::exception& e)
    {
      throw e;
    }
}

// SVG element beginning boilerplate.
// Variable: unit, x=0, y=0, width, height
void
svg_form::start_element()
{
  const std::string start = R"_delimiter_(<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE svg PUBLIC "-//W3C//DTD SVG 1.1//EN" "http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd">
<svg version="1.1"
     id="svg2" xml:space="preserve"
     xmlns:dc="http://purl.org/dc/elements/1.1/"
     xmlns:cc="http://creativecommons.org/ns#"
     xmlns:svg="http://www.w3.org/2000/svg"
     xmlns="http://www.w3.org/2000/svg"
     xmlns:xlink="http://www.w3.org/1999/xlink"
)_delimiter_";

  const std::string unit("__unit");
  const std::string width("__width");
  const std::string height("__height");

  std::string strip = R"_delimiter_(x="0__unit" y="0__unit"
width="__width__unit" height="__height__unit"
viewBox="0 0 __width __height" enable-background="new 0 0 __width __height">
)_delimiter_";

  string_replace(strip, unit, to_string(_M_area._M_unit));
  string_replace(strip, width, std::to_string(_M_area._M_width));
  string_replace(strip, height, std::to_string(_M_area._M_height));

  _M_sstream << start;
  _M_sstream << strip << std::endl;
}


void
svg_form::add_filters()
{
  const std::string f = R"_delimiter_(<defs>
    <filter id="gblur10" x="0" y="0">
      <feGaussianBlur in="SourceGraphic" stdDeviation="10" />
      <feOffset dx="0" dy="0" />
     </filter>
    <filter id="gblur20" x="0" y="0">
      <feGaussianBlur in="SourceGraphic" stdDeviation="20" />
      <feOffset dx="0" dy="0" />
    </filter>
    <filter id="gblur10y" x="0" y="0">
      <feGaussianBlur in="SourceGraphic" stdDeviation="0, 10" />
      <feOffset dx="0" dy="0" />
    </filter>
    <filter id="gblur20y" x="0" y="0">
      <feGaussianBlur in="SourceGraphic" stdDeviation="0, 20" />
      <feOffset dx="0" dy="0" />
    </filter>
  </defs>
)_delimiter_";

  _M_sstream << f << std::endl;
}


// SVG element end boilerplate.
void
svg_form::finish_element()
{
  _M_sstream << "</svg>" << std::endl;
}

void
svg_form::add_title()
{
  _M_sstream << "<title>" << _M_name << "</title>" << std::endl;
}

} // namespace svg

#endif
