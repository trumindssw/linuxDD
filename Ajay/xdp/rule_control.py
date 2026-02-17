import subprocess
import json

RULES = {
    0: "Invalid Ethertype",
    1: "Invalid IP Header Length",
    2: "UDP Length Mismatch",
    3: "UDP Zero Port",
    4: "TCP SYN+FIN",
    5: "SYN Flood"
}

# Default firewall policy
DEFAULT_POLICY = {
    0: 1,
    1: 1,
    2: 1,
    3: 1,
    4: 1,
    5: 1
}

# ---------------- Read Rule State ----------------

def get_rule_states():
    result = subprocess.run(
        ["sudo", "bpftool", "-j", "-p", "map", "dump", "name", "rule_config"],
        capture_output=True,
        text=True
    )

    states = {}

    if result.stdout:
        entries = json.loads(result.stdout)
        for entry in entries:
            if "formatted" in entry:
                states[entry["formatted"]["key"]] = entry["formatted"]["value"]

    return states


# ---------------- Update Rule ----------------

def set_rule(rule_index, value):
    subprocess.run([
        "sudo", "bpftool", "map", "update",
        "name", "rule_config",
        "key", str(rule_index), "0", "0", "0",
        "value", str(value), "0", "0", "0"
    ])


# ---------------- Enable All ----------------

def enable_all():
    for key in RULES:
        set_rule(key, 1)
    print("All rules enabled.")


# ---------------- Disable All ----------------

def disable_all():
    for key in RULES:
        set_rule(key, 0)
    print("All rules disabled.")


# ---------------- Load Default Policy ----------------

def load_default_policy():
    for key, value in DEFAULT_POLICY.items():
        set_rule(key, value)
    print("Default policy loaded.")


# ---------------- Toggle Rule ----------------

def toggle_rule(rule_index, current_state):
    new_state = 0 if current_state == 1 else 1
    set_rule(rule_index, new_state)
    state_str = "Enabled" if new_state == 1 else "Disabled"
    print(f"{RULES[rule_index]} is now {state_str}.")


# ---------------- Main Menu ----------------

while True:
    states = get_rule_states()

    print("\nCurrent Rule Status:")
    for idx, name in RULES.items():
        status = "Enabled" if states.get(idx, 0) == 1 else "Disabled"
        print(f"{idx} - {name} [{status}]")

    print("\nOptions:")
    print("e - Enable all rules")
    print("d - Disable all rules")
    print("l - Load default policy")
    print("q - Quit")

    choice = input("\nSelect rule number to toggle or option: ")

    if choice == "q":
        break

    elif choice == "e":
        enable_all()

    elif choice == "d":
        disable_all()

    elif choice == "l":
        load_default_policy()

    elif choice.isdigit() and int(choice) in RULES:
        rule_index = int(choice)
        toggle_rule(rule_index, states.get(rule_index, 0))

    else:
        print("Invalid choice.")
