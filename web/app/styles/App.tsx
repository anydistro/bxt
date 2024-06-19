/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2023 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
import "./App.css";
import MainPage from "./pages/MainPage";

export default (args: any) => {
    return <MainPage {...args}></MainPage>;
};
