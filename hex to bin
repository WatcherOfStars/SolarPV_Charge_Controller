def hex_bin(hex_str):
    hex_str = hex_str.replace(" ", "")
    binary = []
    for i in range(0, len(hex_str), 2):
        hex = hex_str[i:i+2]
        bin_byte = f"{int(hex, 16):08b}"
        binary.append(bin_byte)
    
    return " ".join(binary)

hex_str0 = input("Enter hex: ")
print(hex_bin(hex_str0))