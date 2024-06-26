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
    user?: User;
    onSaveClicked?: (user: User | undefined, isNew: boolean) => void;
};

export const UserModal = forwardRef<HTMLDialogElement, UserModalProps>(
    ({ user: userProp, onSaveClicked, ...props }, ref) => {
        const [user, setUser] = useState<User>();

        useEffect(() => {
            setUser(
                userProp
                    ? { password: "", permissions: [], ...userProp }
                    : {
                          name: "",
                          password: "",
                          permissions: []
                      }
            );
        }, [userProp]);

        return createPortal(
            <Modal ref={ref} {...props}>
                <Modal.Header>User</Modal.Header>
                <Modal.Body>
                    <Form autoComplete="nope">
                        <Form.Label title="User name" />
                        <Input
                            disabled={userProp != undefined}
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
                                    setUser({
                                        ...user,
                                        password: e.target.value
                                    });
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
                            onSaveClicked?.(user, userProp == undefined)
                        }
                        color="primary"
                    >
                        {userProp == undefined ? "Add" : "Save"}
                    </Button>
                </Modal.Actions>
            </Modal>,
            document.body
        );
    }
);
export default UserModal;
