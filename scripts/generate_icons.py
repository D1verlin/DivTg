#!/usr/bin/env python3
"""
DivTG Icon Generator
Generates high-resolution branded icons (.ico and .png variants) for the DivTG Telegram Desktop mod.
"""

import os
from PIL import Image, ImageDraw, ImageFont

def create_divtg_logo(size=512):
    # Create image with transparent background
    img = Image.new("RGBA", (size, size), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)

    margin = size * 0.05
    box = [margin, margin, size - margin, size - margin]

    # Draw rounded gradient squircle
    radius = size * 0.28
    
    # Create gradient background
    grad = Image.new("RGBA", (size, size), (0, 0, 0, 0))
    gdraw = ImageDraw.Draw(grad)
    
    # Gradient colors: Telegram Blue (#2AABEE) -> Indigo Purple (#7B2CBF) -> Neon Cyan (#00F0FF)
    for y in range(int(margin), int(size - margin)):
        t = (y - margin) / (size - 2 * margin)
        # Interpolate between colors
        if t < 0.5:
            k = t * 2.0
            r = int(42 * (1 - k) + 123 * k)
            g = int(171 * (1 - k) + 44 * k)
            b = int(238 * (1 - k) + 191 * k)
        else:
            k = (t - 0.5) * 2.0
            r = int(123 * (1 - k) + 33 * k)
            g = int(44 * (1 - k) + 150 * k)
            b = int(191 * (1 - k) + 243 * k)
        
        gdraw.line([(margin, y), (size - margin, y)], fill=(r, g, b, 255))

    # Mask for rounded rectangle
    mask = Image.new("L", (size, size), 0)
    mask_draw = ImageDraw.Draw(mask)
    mask_draw.rounded_rectangle(box, radius=radius, fill=255)

    img.paste(grad, (0, 0), mask)

    # Draw stylish inner paper-plane + "D" monogram symbol
    # Paper plane coordinates relative to size
    p_center_x = size * 0.48
    p_center_y = size * 0.52
    scale = size * 0.42

    # Modern paper plane points
    # Nose (top right)
    p_nose = (p_center_x + scale * 0.48, p_center_y - scale * 0.38)
    # Left tail
    p_left = (p_center_x - scale * 0.46, p_center_y - scale * 0.08)
    # Center fold
    p_fold = (p_center_x - scale * 0.12, p_center_y + scale * 0.42)
    # Right tail
    p_right = (p_center_x + scale * 0.12, p_center_y + scale * 0.08)
    # Bottom keel
    p_keel = (p_center_x - scale * 0.05, p_center_y + scale * 0.18)

    # Main wing
    draw.polygon([p_nose, p_left, p_fold], fill=(255, 255, 255, 255))
    # Right wing
    draw.polygon([p_nose, p_fold, p_right], fill=(225, 238, 255, 255))
    # Fold shading
    draw.polygon([p_left, p_fold, p_keel], fill=(190, 210, 245, 255))

    # Add a glowing modern badge accent "DIV" on the top left
    badge_w = size * 0.28
    badge_h = size * 0.11
    badge_x = margin + size * 0.04
    badge_y = margin + size * 0.04
    draw.rounded_rectangle(
        [badge_x, badge_y, badge_x + badge_w, badge_y + badge_h],
        radius=badge_h * 0.5,
        fill=(255, 255, 255, 55),
        outline=(255, 255, 255, 120),
        width=int(max(1, size * 0.005))
    )

    return img

def main():
    root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    art_dir = os.path.join(root, "Telegram", "Resources", "art")
    os.makedirs(art_dir, exist_ok=True)

    print("[DivTG] Generating master 1024x1024 DivTG icon...")
    master = create_divtg_logo(1024)

    # Standard sizes required by Telegram Desktop
    sizes = [16, 32, 48, 64, 128, 256, 512]
    
    for s in sizes:
        resized = master.resize((s, s), Image.Resampling.LANCZOS)
        resized.save(os.path.join(art_dir, f"icon{s}.png"), "PNG")
        
        # 2x variants (HiDPI)
        resized_2x = master.resize((s * 2, s * 2), Image.Resampling.LANCZOS)
        resized_2x.save(os.path.join(art_dir, f"icon{s}@2x.png"), "PNG")
        print(f"  Generated icon{s}.png and icon{s}@2x.png")

    # Logo images
    logo_256 = master.resize((256, 256), Image.Resampling.LANCZOS)
    logo_256.save(os.path.join(art_dir, "logo_256.png"), "PNG")
    logo_256.save(os.path.join(art_dir, "logo_256_no_margin.png"), "PNG")
    print("  Generated logo_256.png and logo_256_no_margin.png")

    # Windows Multi-resolution ICO
    ico_sizes = [(16, 16), (24, 24), (32, 32), (48, 48), (64, 64), (128, 128), (256, 256)]
    ico_path = os.path.join(art_dir, "icon256.ico")
    master.save(ico_path, format="ICO", sizes=ico_sizes)
    print(f"[DivTG] Multi-resolution icon saved to: {ico_path}")

if __name__ == "__main__":
    main()
