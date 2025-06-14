{
  description = "Raylib";

  inputs.nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";

  outputs = { self, nixpkgs }: {
    devShells.x86_64-linux.default = let pkgs = nixpkgs.legacyPackages.x86_64-linux;
    in pkgs.mkShell {
      name = "raylib";
      packages = with pkgs; [
        pkg-config
        xorg.libX11
        xorg.libXi
        xorg.xinput
        xorg.libXinerama
        xorg.libXrandr
        xorg.libXcursor
        xorg.xf86inputevdev
        xorg.xf86videointel
        xorg.xf86inputsynaptics
        xorg.xorgserver
        stdenv.cc.cc.lib
        gcc
        glibc
        glibc.dev
        zlib
        cmake
        gnumake
      ];
    };
  };
}
