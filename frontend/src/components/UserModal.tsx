/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2024 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
import { forwardRef, useEffect, useState } from "react";
import {
    Button,
    Form,
    Input,
    Modal,
    ModalProps,
    Textarea
} from "react-daisyui";
import { createPortal } from "react-dom";

export type UserModalProps = ModalProps & {
    user?: IUser;
    onSaveClicked?: (user: IUser | undefined, isNew: boolean) => void;
};

export default forwardRef<HTMLDialogElement, UserModalProps>((props, ref) => {
    const [user, setUser] = useState<IUser>();

    useEffect(() => {
        setUser(props.user ?? { name: "", password: "", permissions: [] });
    }, [props.user]);

    return createPortal(
        <Modal ref={ref} {...props}>
            <Modal.Header>User</Modal.Header>
            <Modal.Body>
                <Form autoComplete="nope">
                    <Form.Label title="User name" />
                    <Input
                        disabled={props.user != undefined}
                        autoComplete="nope"
                        size="sm"
                        value={user?.name}
                        placeholder="Enter user name"
                        onChange={(e) =>
                            setUser({ ...user, name: e.target.value })
                        }
                        type="text"
                        className="input-bordered"
                    />
                    <Form.Label title="Password" />
                    <Input
                        autoComplete="new-password"
                        size="sm"
                        value={user?.password}
                        placeholder="Enter new password"
                        onChange={(e) => {
                            if (user)
                                setUser({ ...user, password: e.target.value });
                        }}
                        type="password"
                        className="input-bordered"
                    />
                    <Form.Label title="Permissions" />
                    <Textarea
                        value={user?.permissions?.join("\n")}
                        onChange={(e) => {
                            setUser({
                                ...user!,
                                permissions: e.target.value.split("\n")
                            });
                        }}
                    />
                </Form>
            </Modal.Body>
            <Modal.Actions>
                <Button
                    size="sm"
                    onClick={() =>
                        props.onSaveClicked?.(user, props.user == undefined)
                    }
                    color="primary"
                >
                    {props.user == undefined ? "Add" : "Save"}
                </Button>
            </Modal.Actions>
        </Modal>,
        document.body
    );
});
