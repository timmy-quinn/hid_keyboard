#experimental

import json
import os

# Paths
compile_commands_path = "build/compile_commands.json"
vscode_workspace_path = "zephyr_project.code-workspace"

# Load compile_commands.json
with open(compile_commands_path, "r") as f:
    compile_commands = json.load(f)

# Extract unique directories
directories = sorted(set(entry["directory"] for entry in compile_commands))

# Create workspace structure
workspace = {"folders": [{"path": "."}]}
for directory in directories:
    workspace["folders"].append({"path": directory})

# Write the .code-workspace file
with open(vscode_workspace_path, "w") as f:
    json.dump(workspace, f, indent=4)

print(f"Workspace updated: {vscode_workspace_path}")