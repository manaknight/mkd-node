// Hello World test program
effect log

api get "/hello" () -> String {
    log.info("Hello endpoint called")
    "Hello from Manaknight API!"
}

fn main() -> String {
    log.info("Main function called")
    "Hello, World!"
}
