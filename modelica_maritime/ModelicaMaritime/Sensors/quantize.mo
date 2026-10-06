within ModelicaMaritime.Sensors;
function quantize "Round a scalar to the nearest quantization interval"
  input Real value;
  input Real interval;
  output Real quantized;
algorithm
  assert(interval >= 0, "Quantization interval must be non-negative");
  quantized := if interval > 0 then
    floor(value / interval + 0.5) * interval else value;
end quantize;
