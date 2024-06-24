/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2023 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
import FileViewPage from "./FileViewPage";
import { RouterProvider, createBrowserRouter } from "react-router-dom";
import LogPage from "./LogPage";
import { ToastContainer, toast } from "react-toastify";
import { useLocalStorage } from "@uidotdev/usehooks";
import LoginPage from "./LoginPage";
import ComparePage from "./ComparePage";
import DrawerLayout from "../components/DrawerLayout";
import axios, { AxiosError } from "axios";
import AdminPage from "./AdminPage";

import axiosRetry, { isNetworkOrIdempotentRequestError } from "axios-retry";

declare module "@uidotdev/usehooks" {
    export function useLocalStorage<T>(
        key: string,
        initialValue: T
    ): [T, (v: T) => void];
}
export default (props: any) => {
    const [userName, setUserName] = useLocalStorage("username", null);

    axios.defaults.withCredentials = true;
    axiosRetry(axios, {
        retries: 3,
        retryDelay: axiosRetry.exponentialDelay,
        retryCondition: (error) =>
            isNetworkOrIdempotentRequestError(error) ||
            error.response?.status === 401,
        onRetry: async (retryCount, error, requestConfig) => {
            (error.config as any)._retry = true;
            try {
                const instance = axios.create();
                const response = await instance.get("/api/auth/refresh");

                return Promise.resolve();
            } catch (refreshError) {
                toast.error("Error " + error);
                setUserName(null);

                // Dismiss existing toasts to avoid duplicate error messages
                toast.dismiss();
                toast.error(
                    "You are not authorized to access this page, try logging in again.",
                    {
                        autoClose: false
                    }
                );
                return Promise.reject();
            }
        }
    });
    axios.interceptors.response.use(
        (response) => response,
        (error) => {
            if (error.response?.status !== 401) {
                toast.error(
                    `Response error: ${error.response?.data?.message}`,
                    {
                        autoClose: false
                    }
                );
                return Promise.resolve(error);
            }
        }
    );
    const router = createBrowserRouter([
        {
            element: <DrawerLayout />,
            children: [
                {
                    path: "",
                    element: <FileViewPage style={{ zIndex: 10 }} />
                },
                {
                    path: "logs",
                    element: <LogPage />
                },
                {
                    path: "compare",
                    element: <ComparePage />
                },
                {
                    path: "admin",
                    element: <AdminPage />
                }
            ]
        }
    ]);

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
