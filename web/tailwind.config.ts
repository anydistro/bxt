import type { Config } from "tailwindcss";
import daisyui from "daisyui";

export default {
    content: [
        "./app/**/*.{js,jsx,ts,tsx}",
        "node_modules/daisyui/dist/**/*.js",
        "node_modules/react-daisyui/dist/**/*.js"
    ],
    theme: {
        extend: {}
    },
    daisyui: {
        themes: [
            {
                carton: {
                    primary: "#B09E8F",
                    secondary: "#8B756B",
                    accent: "#6D564C",
                    neutral: "#C7B9B2",
                    "base-100": "#F3F3F3",
                    "base-content": "#4C3C34",
                    info: "#536dfe",
                    success: "#4CAF50",
                    warning: "#FFC107",
                    error: "#FF5252"
                }
            }
        ]
    },
    plugins: [daisyui]
} satisfies Config;
