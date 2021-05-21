use std::process::Command;

fn main() {
    // Query HEAD revision and expose as $TC_GIT_REV during build
    //
    // Adapted from https://stackoverflow.com/questions/43753491
    let cmd = Command::new("git")
        .args(&["rev-parse", "--short", "HEAD"])
        .spawn()
        // Wait for process to exit
        .and_then(|cmd| cmd.wait_with_output())
        // Handle error if failed to launch git
        .map_err(|_e| println!("cargo:warning=Failed to run 'git' to determine HEAD rev"))
        // Remap to Some/None for simpler error handling
        .ok()
        // Handle command failing
        .and_then(|o| {
            if o.status.success() {
                Some(o)
            } else {
                println!(
                    "cargo:warning='git' exited with non-zero exit code while determining HEAD rev"
                );
                None
            }
        })
        // Get output as UTF-8 string
        .map(|out| String::from_utf8(out.stdout).expect("Invalid output in stdout"));

    // Only output git rev if successful
    if let Some(h) = cmd {
        println!("cargo:rustc-env=TC_GIT_REV={}", h);
    }
}
