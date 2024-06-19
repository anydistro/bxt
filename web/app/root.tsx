import {
    Link,
    Links,
    Meta,
    Outlet,
    Scripts,
    ScrollRestoration,
    redirect,
    useLocation,
    useNavigate,
    useRevalidator
} from "@remix-run/react";
import "./tailwind.css";
import {
    faFolderTree,
    faCodeCompare,
    faListCheck,
    faToolbox,
    faCircleDown,
    faRightFromBracket
} from "@fortawesome/free-solid-svg-icons";
import { FontAwesomeIcon } from "@fortawesome/react-fontawesome";

import axios from "axios";
import { useRef, useCallback, useMemo, useEffect, useState } from "react";
import { Drawer, Menu, Progress, Button } from "react-daisyui";
import ConfirmSyncModal from "./components/ConfirmSyncModal";
import { LinksFunction } from "@remix-run/node";
import axiosRetry, { isNetworkOrIdempotentRequestError } from "axios-retry";
import { ToastContainer, toast } from "react-toastify";
import LoginForm from "./components/LoginForm";
import { useSyncMessage } from "./hooks/BxtWebSocketHooks";
import { ClientOnly } from "remix-utils/client-only";

export function useLocalStorage<T>(
    key: string
): [T | null | undefined, (state: T | null) => void] {
    const [state, setState] = useState<T | null | undefined>(undefined);

    useEffect(() => {
        const value = localStorage.getItem(key);
        const state: T = value != null ? JSON.parse(value) : null;

        setState(state);
    }, [key, setState]);

    const setWithLocalStorage = (nextState: T | null) => {
        localStorage.setItem(key, JSON.stringify(nextState));
        setState(nextState);
    };

    return [state, setWithLocalStorage];
}
export const links: LinksFunction = () => {
    return [
        {
            rel: "icon",
            href: "/logo192.png",
            type: "image/png"
        }
    ];
};

export function Layout({ children }: { children: React.ReactNode }) {
    return (
        <html lang="en">
            <head>
                <meta charSet="utf-8" />
                <meta
                    name="viewport"
                    content="width=device-width, initial-scale=1"
                />
                <Meta />
                <Links />
            </head>
            <body>
                {children}
                <ScrollRestoration />
                <Scripts />
            </body>
        </html>
    );
}

const triggerSync = async () => {
    await axios.post("/api/packages/sync");
};

export default function App() {
    const routes = useMemo(
        () => [
            { route: "/", name: "Packages", icon: faFolderTree },
            { route: "/compare", name: "Compare", icon: faCodeCompare },
            { route: "/logs", name: "Logs", icon: faListCheck },
            { route: "/admin", name: "Admin", icon: faToolbox }
        ],
        []
    );

    const location = useLocation();
    const navigate = useNavigate();
    const [userName, setUserName] = useLocalStorage<string>("username");
    const syncInProgress = useSyncMessage()?.started;

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
    let modalRef = useRef<HTMLDialogElement>(null);
    const handleShow = useCallback(() => {
        modalRef.current?.showModal();
    }, []);

    if (userName == null) {
        return <LoginForm setUserName={setUserName} />;
    }

    return (
        <Drawer
            className="lg:drawer-open"
            open={true}
            side={
                <Menu className="h-screen p-4 w-60 bg-base-100 text-base-content border-r-base border-r-2">
                    <Link
                        to="/"
                        className="flex justify-center relative  h-14  overflow-hidden pb-2/3"
                    >
                        <img
                            id="bxt-logo"
                            src={`logo-full.png`}
                            className="absolute h-full w-40 object-contain"
                        />
                    </Link>

                    <div className="h-6"></div>

                    {routes.map(({ route, name, icon }) => (
                        <Menu.Item>
                            <Link
                                className={
                                    location.pathname == route ? "active" : ""
                                }
                                to={route}
                            >
                                <FontAwesomeIcon icon={icon} />
                                {name}
                            </Link>
                        </Menu.Item>
                    ))}

                    <div className="grow"></div>
                    <ClientOnly>
                        {() =>
                            syncInProgress ? (
                                <div className="font-bold text-center">
                                    Sync is in progress
                                    <Progress />
                                </div>
                            ) : (
                                <Button
                                    size="sm"
                                    onClick={handleShow}
                                    color="accent"
                                >
                                    <FontAwesomeIcon icon={faCircleDown} />
                                    Sync
                                </Button>
                            )
                        }
                    </ClientOnly>
                    <div className="h-5 flex flex-col place-content-center">
                        <hr />
                    </div>

                    <Menu.Item onClick={() => setUserName(null)}>
                        <a>
                            <FontAwesomeIcon icon={faRightFromBracket} />
                            Logout
                        </a>
                    </Menu.Item>
                </Menu>
            }
        >
            <ClientOnly>
                {() => (
                    <ConfirmSyncModal
                        onCancel={() => modalRef.current?.close()}
                        onConfirm={() => {
                            triggerSync();
                            modalRef.current?.close();
                        }}
                        ref={modalRef}
                    />
                )}
            </ClientOnly>
            <ToastContainer />
            <Outlet />
        </Drawer>
    );
}
