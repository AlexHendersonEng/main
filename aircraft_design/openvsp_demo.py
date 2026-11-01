import openvsp as vsp
import os
import sys
import traceback

print("OpenVSP Python API Test")

try:
    # Check Python module import
    print("\n[1] Import successful!")

    # Print OpenVSP version
    try:
        version = vsp.GetVSPVersion()
        print(f"[2] OpenVSP Version: {version}")
    except Exception:
        print("[2] Could not retrieve version.")

    # Initialise OpenVSP
    vsp.VSPRenew()
    print("[3] OpenVSP initialised successfully!")

    # Create a wing
    wing_id = vsp.AddGeom("WING", "")
    print(f"[4] Wing created! ID: {wing_id}")

    # Set wing parameters
    vsp.SetParmVal(wing_id, "Span", "XSec_1", 10.0)
    vsp.SetParmVal(wing_id, "Root_Chord", "XSec_1", 2.0)
    vsp.SetParmVal(wing_id, "Tip_Chord", "XSec_1", 1.0)

    print("[5] Wing parameters configured!")

    # Update model
    vsp.Update()
    print("[6] Model updated successfully!")

    # Save model
    output_file = os.path.abspath("openvsp_test.vsp3")
    vsp.WriteVSPFile(output_file)

    print(f"[7] Model saved to:\n    {output_file}")

    # Verify output
    if os.path.exists(output_file):
        print("[8] Output file verified!")

    print("SUCCESS: OpenVSP Python API is working!")

except Exception as e:
    print("FAILED: OpenVSP Python API test encountered an error.")
    print(f"\nError: {e}")
    traceback.print_exc()
    sys.exit(1)
