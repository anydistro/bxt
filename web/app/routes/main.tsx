/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2023 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */

import { RouterProvider, createBrowserRouter } from "react-router-dom";

import { ToastContainer, toast } from "react-toastify";
import { useLocalStorage } from "@uidotdev/usehooks";

import DrawerLayout from "../components/DrawerLayout";
import axios, { AxiosError } from "axios";

import axiosRetry, { isNetworkOrIdempotentRequestError } from "axios-retry";

export default (props: any) => {
    return (
        <div className="flex w-full items-center justify-center font-sans">
            <ToastContainer />

            {userName != null ? (
                <RouterProvider router={router} />
            ) : (
                <LoginPage />
            )}
        </div>
    );
};
