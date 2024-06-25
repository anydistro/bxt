/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2024 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */

import { ChonkyFileActionData } from "chonky";

export type ActionHandler = (payload: ChonkyFileActionData) => void;

export type ActionHandlers = [actionId: string, actionHandler: ActionHandler][];
