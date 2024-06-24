/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2023 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
import { useLocalStorage } from "@uidotdev/usehooks";
import axios from "axios";
import { useCallback, useState } from "react";
import { Hero, Button, Card, Form, Input } from "react-daisyui";
import { toast } from "react-toastify";

export default (props: any) => {
    const [token, setToken] = useState("");
    const [password, setPassword] = useState("");
    const [name, setName] = useState("");

    const [userName, setUserName] = useLocalStorage<string | null>(
        "username",
        null
    );

    const authentificateClicked = useCallback(async () => {
        const result = await axios
            .post("/api/auth", {
                name: name,
                password: password,
                response_type: "cookie"
            })
            .catch((err) => {
                toast.error("Login failed");

                return Promise.reject(err);
            });
        if (result.status == 200) {
            setUserName(name);
            toast.done("Login sucessful");
        }
    }, [name, password, setUserName]);

    const [showPassword, setShowPassword] = useState<boolean>(false);
    const switchShowPassword = useCallback(() => {
        setShowPassword(!showPassword);
    }, [showPassword, setShowPassword]);

    return (
        <div
            style={{
                backgroundImage: `url(/background.png)`
            }}
            className="bg-cover flex w-full component-preview p-4 items-center justify-center gap-2 font-sans"
        >
            <Hero className="grid h-screen place-items-center" {...props}>
                <Hero.Content className="flex-col lg:flex-row-reverse">
                    <Card className="flex-shrink-0 w-full max-w-sm shadow-2xl bg-base-100">
                        <Card.Body>
                            <div className="justify-start relative h-12 overflow-hidden pb-2/3">
                                <img
                                    src={`/logo-full.png`}
                                    className="absolute h-full w-full object-contain"
                                />
                            </div>
                            <Form
                                onSubmit={(e) => {
                                    e.preventDefault();
                                    authentificateClicked();
                                }}
                            >
                                <Form.Label title="Login" />
                                <Input
                                    value={name}
                                    onChange={(e) => setName(e.target.value)}
                                    type="text"
                                    placeholder="login"
                                    className="input-bordered"
                                />
                                <Form.Label title="Password" />
                                <Input
                                    value={password}
                                    onChange={(e) =>
                                        setPassword(e.target.value)
                                    }
                                    type={showPassword ? "text" : "password"}
                                    placeholder="password"
                                    className="input-bordered"
                                ></Input>
                                <Form.Label />
                                <Button
                                    type="submit"
                                    value="submit"
                                    color="primary"
                                >
                                    Login
                                </Button>
                            </Form>
                        </Card.Body>
                    </Card>
                </Hero.Content>
            </Hero>
        </div>
    );
};
