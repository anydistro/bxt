/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2023 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
import { Drawer, Menu, Button, Progress } from "react-daisyui";
import { Link, Outlet, useLocation } from "react-router-dom";
import ConfirmSyncModal from "../modals/ConfirmSyncModal";
import { useCallback, useMemo, useRef, useState } from "react";
import axios from "axios";
import { useSyncMessage } from "../hooks/BxtWebSocketHooks";
import { FontAwesomeIcon } from "@fortawesome/react-fontawesome";
import {
    faBars,
    faCircleDown,
    faCodeCompare,
    faFolderTree,
    faListCheck,
    faRightFromBracket,
    faToolbox,
    faUser
} from "@fortawesome/free-solid-svg-icons";
import useLocalStorage from "../hooks/useLocalStorage";

const triggerSync = async (architecture: string) => {
    await axios.post("/api/packages/sync", {
        architecture
    });
};

export default function RootDrawerLayout() {
    let modalRef = useRef<HTMLDialogElement>(null);
    const [userName, setUserName] = useLocalStorage("username", null);

    const revokeToken = async () => {
        await axios.post("/api/auth/revoke");
        setUserName(null);
    };

    const syncInProgress = useSyncMessage()?.started;

    const [syncArchitecture, setSyncArchitecture] = useState("x86_64");

    const handleShow = useCallback(
        (syncArchitecture: string) => {
            setSyncArchitecture(syncArchitecture);
            modalRef.current?.showModal();
        },
        [modalRef]
    );

    const routes = useMemo(
        () => [
            { route: "/", name: "Packages", icon: faFolderTree },
            { route: "/compare", name: "Compare", icon: faCodeCompare },
            { route: "/log", name: "Log", icon: faListCheck },
            { route: "/admin", name: "Admin", icon: faToolbox }
        ],
        []
    );

    const location = useLocation();

    const [drawerOpened, setDrawerOpened] = useState(false);

    return (
        <Drawer
            className="lg:drawer-open"
            open={drawerOpened}
            onClickOverlay={() => setDrawerOpened(false)}
            side={
                <Menu className="h-screen p-4 w-60 bg-base-100 text-base-content border-r-base border-r-2">
                    <Link
                        to="/"
                        className="flex justify-center relative  h-14  overflow-hidden pb-2/3"
                    >
                        <img
                            id="bxt-logo"
                            src={`/logo-full.svg`}
                            className="absolute h-full w-40 object-contain"
                        />
                    </Link>

                    <div className="h-6"></div>

                    {routes.map(({ route, name, icon }) => (
                        <Menu.Item key={route}>
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
                    <div className="font-bold text-center">Sync</div>

                    {syncInProgress ? (
                        <div className="font-bold text-center">
                            Sync is in progress
                            <Progress />
                        </div>
                    ) : (
                        <div className="flex items-center gap-2">
                            <Button
                                size="sm"
                                onClick={() => handleShow("aarch64")}
                                color="accent"
                            >
                                <FontAwesomeIcon icon={faCircleDown} />
                                aarch64
                            </Button>
                            <Button
                                size="sm"
                                onClick={() => handleShow("x86_64")}
                                color="accent"
                            >
                                <FontAwesomeIcon icon={faCircleDown} />
                                x86_64
                            </Button>
                        </div>
                    )}

                    <div className="h-5 flex flex-col place-content-center">
                        <hr />
                    </div>

                    <span className="flex items-center justify-between">
                        <span>
                            <FontAwesomeIcon icon={faUser} className="mr-2" />
                            {userName}
                        </span>
                        <Menu.Item>
                            <a onClick={(e) => revokeToken()}>
                                <FontAwesomeIcon icon={faRightFromBracket} />
                                Logout
                            </a>
                        </Menu.Item>
                    </span>
                </Menu>
            }
        >
            <ConfirmSyncModal
                onCancel={() => modalRef.current?.close()}
                onConfirm={() => {
                    triggerSync(syncArchitecture);
                    modalRef.current?.close();
                }}
                architecture={syncArchitecture}
                ref={modalRef}
            />
            <Button
                className={`fixed bottom-6 left-6 z-20 lg:hidden ${drawerOpened ? "hidden" : ""}`}
                onClick={() => setDrawerOpened(!drawerOpened)}
                color="primary"
                size="md"
            >
                <FontAwesomeIcon icon={faBars} />
            </Button>
            <div className="flex h-screen">
                <Outlet />
            </div>
        </Drawer>
    );
}
