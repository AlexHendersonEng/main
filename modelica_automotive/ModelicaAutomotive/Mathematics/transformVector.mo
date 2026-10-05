within ModelicaAutomotive.Mathematics;
function transformVector "Apply a three-dimensional direction-cosine matrix"
  input ModelicaAutomotive.Types.Matrix3 dcm;
  input ModelicaAutomotive.Types.Vector3 vector;
  output ModelicaAutomotive.Types.Vector3 transformed;
algorithm
  transformed := dcm * vector;
end transformVector;
