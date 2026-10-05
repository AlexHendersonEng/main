within ModelicaAutomotive.Mathematics;
function roadToWorldMatrix "Direction-cosine matrix from road axes to world axes"
  input ModelicaAutomotive.Types.Angle heading;
  input ModelicaAutomotive.Types.Angle grade;
  input ModelicaAutomotive.Types.Angle bank;
  output ModelicaAutomotive.Types.Matrix3 dcm;
protected
  Real ch;
  Real sh;
  Real cg;
  Real sg;
  Real cb;
  Real sb;
algorithm
  ch := cos(heading);
  sh := sin(heading);
  cg := cos(grade);
  sg := sin(grade);
  cb := cos(bank);
  sb := sin(bank);
  dcm := [
    ch * cg, -sh * cb - ch * sg * sb, sh * sb - ch * sg * cb;
    sh * cg, ch * cb - sh * sg * sb, -ch * sb - sh * sg * cb;
    sg, cg * sb, cg * cb];
end roadToWorldMatrix;
