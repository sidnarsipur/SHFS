use tonic::{transport::Server, Request, Response, Status};

use storage_server::storage_service_server::{StorageService, StorageServiceServer};
use storage_server::{RegisterReply, RegisterRequest};

pub mod storage_server {
    tonic::include_proto!("storage_server");
}

#[derive(Debug, Default)]
pub struct StorageServiceImpl {}

#[tonic::async_trait]
impl StorageService for StorageServiceImpl {
    async fn register(
        &self,
        request: Request<RegisterRequest>,
    ) -> std::result::Result<Response<RegisterReply>, Status> {
        println!("Register storage server called: {:?}", request);

        let reply = RegisterReply { confirmed: true };

        Ok(Response::new(reply))
    }
}

pub fn create(server_name: &str) {
    println!("Storage server {} running...", server_name);

    // Start the gRPC server
    tokio::spawn(async move {
        let addr = "[::1]:50051".parse().expect("Invalid address");
        let storage_service = StorageServiceImpl::default();

        if let Err(e) = Server::builder()
            .add_service(StorageServiceServer::new(storage_service))
            .serve(addr)
            .await
        {
            eprintln!("Failed to start gRPC server: {}", e);
        }
    });

    // Busy waiting loop to keep the function running
    loop {
        std::thread::sleep(std::time::Duration::from_secs(1));
    }
}
