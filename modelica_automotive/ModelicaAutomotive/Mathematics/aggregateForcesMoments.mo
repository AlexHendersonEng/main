within ModelicaAutomotive.Mathematics;
function aggregateForcesMoments
  "Aggregate four applied forces and their moments about a reference point"
  input Real forces[4, 3];
  input Real applicationPoints[4, 3];
  output Real totalForce[3];
  output Real totalMoment[3];
algorithm
  totalForce := {0, 0, 0};
  totalMoment := {0, 0, 0};
  for corner in 1:4 loop
    totalForce := totalForce + forces[corner, :];
    totalMoment[1] := totalMoment[1]
      + applicationPoints[corner, 2] * forces[corner, 3]
      - applicationPoints[corner, 3] * forces[corner, 2];
    totalMoment[2] := totalMoment[2]
      + applicationPoints[corner, 3] * forces[corner, 1]
      - applicationPoints[corner, 1] * forces[corner, 3];
    totalMoment[3] := totalMoment[3]
      + applicationPoints[corner, 1] * forces[corner, 2]
      - applicationPoints[corner, 2] * forces[corner, 1];
  end for;
end aggregateForcesMoments;
