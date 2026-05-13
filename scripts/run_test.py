import subprocess
import os
import sys

EXE_PATH = "./build/bch_test"
# List of m values for the scalability analysis
M_VALUES = [7, 9, 11, 13, 15]
TARGET_EBNO_DB = 5.0

def run_scalability_test():
    os.makedirs("results/csv", exist_ok=True)

    if not os.path.exists(EXE_PATH):
        print(f"❌ Executable not found: {EXE_PATH}. Build first with `make -j`.")
        sys.exit(1)

    print(f"🔬 Starting scalability test at Eb/N0 = {TARGET_EBNO_DB} dB")
    
    for m in M_VALUES:
        # Scale t so m=15 does not take forever,
        # but still remains a real challenge for the code.
        t = 10 if m < 10 else 50 if m < 13 else 100
        
        print(f"  > Testing m={m} (n={2**m-1}), t={t}...", end=" ", flush=True)
        
        # Run a single point to collect average encoding/decoding times.
        cmd = [EXE_PATH, str(m), str(t), str(TARGET_EBNO_DB), str(TARGET_EBNO_DB), "1"]
        
        try:
            result = subprocess.run(cmd, capture_output=True, text=True, check=True)
            if result.stdout:
                last_line = result.stdout.strip().splitlines()[-1]
                print(f"✅ {last_line}")
            else:
                print("✅")
        except Exception as e:
            print(f"❌ Error at m={m}: {e}")
            if isinstance(e, subprocess.CalledProcessError) and e.stderr:
                print(e.stderr)

if __name__ == "__main__":
    run_scalability_test()