/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2024 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
import axios from "axios";
import { useCallback, useEffect, useState } from "react";

export const useUsers = (): [IUser[] | undefined, () => void] => {
    const [users, setUsers] = useState<IUser[]>();

    const updateUsers = useCallback(async () => {
        const users = await axios.get<IUser[]>("/api/users");
        if (users) setUsers(users.data);
    }, [setUsers]);

    useEffect(() => {
        updateUsers();
    }, []);

    return [users, updateUsers];
};
