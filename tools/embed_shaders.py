from pathlib import Path

Path(repo = Path(__file__).resolve().parent.parent)

jobs = [
    (
        repo / "x64" / "Debug" / "GenerateMips_CS.cso",
        repo / "dx12lib" / "GenerateMips_CS.h",
        "g_GenerateMips_CS",
    ),
    (
        repo / "x64" / "Debug" / "PanoToCubemap_CS.cso",
        repo / "dx12lib" / "PanoToCubemap_CS.h",
        "g_PanoToCubemap_CS",
    ),
]

for cso_path, header_path, variable_name in jobs:
    data = cso_path.read_bytes()

    lines = []
    for i in range(0, len(data), 12):
        chunk = data[i:i + 12]
        lines.append(
            "    " + ", ".join(f"0x{byte:02X}" for byte in chunk)
        )

    header = (
        "#pragma once\n\n"
        f"const unsigned char {variable_name}[] =\n"
        "{\n"
        + ",\n".join(lines)
        + "\n};\n"
    )

    header_path.write_text(header, encoding="utf-8")

    print(f"Generated: {header_path}")
    print(f"Bytes: {len(data)}")