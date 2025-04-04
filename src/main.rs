use clap::{Parser, Subcommand};
mod client;
mod naming;
mod storage;

#[derive(Parser)]
#[command(version, about, long_about = None)]
struct Cli {
    #[command(subcommand)]
    command: Option<Commands>,
}

#[derive(Subcommand)]
enum Commands {
    Naming {},
    Storage {},
    Client {},
}

fn main() {
    let cli = Cli::parse();

    match &cli.command {
        Some(Commands::Naming {}) => naming::run(),
        Some(Commands::Storage {}) => storage::run(),
        Some(Commands::Client {}) => client::run(),
        None => println!("Welcome to DFS. Use -h to see usage."),
    };
}
