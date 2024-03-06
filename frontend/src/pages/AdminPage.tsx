/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2024 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
import { faPlus, faTrash } from "@fortawesome/free-solid-svg-icons";
import { FontAwesomeIcon } from "@fortawesome/react-fontawesome";
import { useCallback, useRef, useState } from "react";
import { Table, Button } from "react-daisyui";
import { useUsers } from "../hooks/AdminHooks";
import UserModal, { UserModalProps } from "../components/UserModal";
import axios from "axios";
export default () => {
    const [users, reloadUsers] = useUsers();

    const userModalRef = useRef<HTMLDialogElement>(null);

    const [userModalProps, setUserModalProps] = useState<UserModalProps>({
        backdrop: true
    });

    const openUserModal = useCallback(
        (user?: IUser) => {
            setUserModalProps({ ...userModalProps, user });

            userModalRef.current?.showModal();
        },
        [userModalProps, setUserModalProps, userModalRef]
    );

    const removeUser = useCallback(
        async (user: IUser) => {
            await axios.post("/api/users/remove", { id: user.name });
            reloadUsers();
        },
        [reloadUsers]
    );

    const addUser = useCallback(
        async (user: IUser) => {
            await axios.post("/api/users/add", user);
            reloadUsers();
        },
        [reloadUsers]
    );

    const updateUser = useCallback(
        async (user: IUser) => {
            await axios.post("/api/users/update", user);
            reloadUsers();
        },
        [reloadUsers]
    );

    return (
        <div className="px-2 w-full h-full">
            <UserModal
                onSaveClicked={(user, isNew) => {
                    if (user) {
                        if (isNew) {
                            addUser(user);
                        } else {
                            updateUser(user);
                        }
                    }
                    userModalRef.current?.close();
                }}
                {...userModalProps}
                ref={userModalRef}
            />
            <Table zebra={true} className="rounded-none">
                <thead>
                    <th style={{ width: "90%" }}>Name</th>
                    <th style={{ width: "10%" }}></th>
                </thead>
                <Table.Body>
                    {users?.map((value) => (
                        <Table.Row
                            style={{ userSelect: "none" }}
                            onDoubleClick={() => openUserModal(value)}
                        >
                            <span>{value.name}</span>

                            <Button
                                color="ghost"
                                onClick={() => removeUser(value)}
                            >
                                <FontAwesomeIcon icon={faTrash} />
                            </Button>
                        </Table.Row>
                    ))}
                </Table.Body>
            </Table>
            <div className="fixed bottom-6 right-6 space-x-4">
                <Button color="accent" onClick={() => openUserModal()}>
                    <FontAwesomeIcon icon={faPlus} />
                    New user
                </Button>
            </div>
        </div>
    );
};
