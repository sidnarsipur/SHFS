import subprocess
import os
import time
import shutil

# Paths to your executables (adjust if needed)
BUILD_DIR = "./cmake-build-debug-visual-studio/src/"
CLIENT_EXEC = os.path.join(BUILD_DIR, "client", "client.exe")
NAMING_EXEC = os.path.join(BUILD_DIR, "naming", "naming.exe")
STORAGE_EXEC = os.path.join(BUILD_DIR, "storage", "storage.exe")


def run_naming(*args):
    """port, timeout, replication factor"""
    full_args = [NAMING_EXEC] + list(map(str, args))
    process = subprocess.Popen(full_args)
    print(f"Naming server started (PID {process.pid})")
    return process


def run_storage(port, folder):
    folder = os.path.join("test", folder)
    os.makedirs(folder, exist_ok=True)
    process = subprocess.Popen([STORAGE_EXEC, str(port)], cwd=folder)
    print(f"Storage server started on port {port} in folder '{folder}' (PID {process.pid})")
    return process


def kill_storage(process):
    process.terminate()
    try:
        process.wait(timeout=5)
        print(f"Storage server (PID {process.pid}) terminated gracefully.")
    except subprocess.TimeoutExpired:
        process.kill()
        print(f"Storage server (PID {process.pid}) killed forcefully.")


def run_client(*args):
    full_args = [CLIENT_EXEC] + list(map(str, args))
    subprocess.run(full_args)
    print(f"Client run with arguments: {args}")


def spawn_storage_servers(start_port, folder_prefix, count):
    processes = []
    for i in range(count):
        port = start_port + i
        folder = f"{folder_prefix}{i+1}"
        proc = run_storage(port, folder)
        processes.append(proc)
    return processes


def create_file(filename):
    with open(filename, 'w') as f:
        f.write("hello world")


def cleanup():
    shutil.rmtree("test", ignore_errors=True)  # Remove 'test' directory and contents


def main():
    cleanup()
    naming_proc = run_naming(6000, 10, 2)
    time.sleep(2)  # Wait for naming to be ready

    # Start multiple storage servers
    storage_processes = spawn_storage_servers(start_port=9000, folder_prefix="storage", count=2)
    time.sleep(2)  # Wait for storage servers to be ready

    # Run client
    create_file("file.txt")  # Create a test file
    run_client("upload", "file.txt")  # Example client command

    new_storage_processes = run_storage(9002, "storage3")  # Start a new storage server

    time.sleep(5)  # Let system run for a bit

    # Kill one storage server by index (e.g., kill the second one)
    kill_storage(storage_processes[1])

    time.sleep(100)  # Observe system after killing a storage server

    # Cleanup remaining storage servers
    for proc in storage_processes:
        if proc.poll() is None:  # Only terminate if still running
            proc.terminate()
    naming_proc.terminate()
    new_storage_processes.terminate()


if __name__ == "__main__":
    main()
