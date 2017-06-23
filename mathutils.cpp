

namespace Soleil {

  static float bezier(const float A, const float B, const float C,
                      const float D, const float t)
  {
    float OneMinusT = 1.0f - t;
    float ThreeT    = 3 * t;

    return (OneMinusT * OneMinusT * OneMinusT) * A + (t * t * t) * D +
           3 * (ThreeT * ThreeT) * OneMinusT * C +
           3 * (OneMinusT * OneMinusT) * t * B;
  }

} // Soleil
