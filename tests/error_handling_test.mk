// This file should cause compilation errors
// Testing error handling (currently the compiler doesn't validate much)

fn missing_return_type() {
    "This should fail"
}

fn incomplete_function() -> String {
    // Missing closing brace

fn valid_function() -> String {
    "This should work despite errors above"
}

fn main() -> String {
    "Error handling test"
}
