within ModelicaAutomotive.Mathematics;
function quaternionMultiply "Hamilton product of scalar-first quaternions"
  input ModelicaAutomotive.Types.Quaternion left;
  input ModelicaAutomotive.Types.Quaternion right;
  output ModelicaAutomotive.Types.Quaternion product;
algorithm
  product[1] :=
    left[1] * right[1] - left[2] * right[2]
    - left[3] * right[3] - left[4] * right[4];
  product[2] :=
    left[1] * right[2] + left[2] * right[1]
    + left[3] * right[4] - left[4] * right[3];
  product[3] :=
    left[1] * right[3] - left[2] * right[4]
    + left[3] * right[1] + left[4] * right[2];
  product[4] :=
    left[1] * right[4] + left[2] * right[3]
    - left[3] * right[2] + left[4] * right[1];
end quaternionMultiply;
