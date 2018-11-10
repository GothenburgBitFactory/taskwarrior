use std::collections::HashMap;

#[derive(Debug)]
pub struct Task {
    data: HashMap<String, String>,
}

impl Task {
    /// Construct a Task from a hashmap containing named properties
    pub fn from_data(data: HashMap<String, String>) -> Self {
        Self { data }
    }

    pub fn description(&self) -> &str {
        self.data.get("description").unwrap()
    }
}
